#include "flexbridge.h"
#include <QCoreApplication>
#include <QFile>
#include <QtEndian>
#include <cmath>
#include <iostream>
#include <numbers>
#include <vector>

int main(int argc,char** argv){
 QCoreApplication app(argc,argv);
 QFile fixture(QStringLiteral(FLEX_FIXTURE));
 if(!fixture.open(QIODevice::ReadOnly))return 1;
 auto pcm=qUncompress(fixture.readAll());
 if(pcm.isEmpty()||pcm.size()%2)return 1;
 constexpr int iqRate=240000,audioRate=22050;
 // Modulate the reference PCM entirely in memory. No transmitter is involved.
 QByteArray iq;
 double phase=0;
 for(qint64 n=0;n*audioRate/iqRate<pcm.size()/2;++n){
  auto sample=qFromLittleEndian<qint16>(pcm.constData()+2*(n*audioRate/iqRate));
  phase+=2*std::numbers::pi*4500*sample/(32768.0*iqRate);
  iq.append(char(std::lround(127.5+110*std::cos(phase))));
  iq.append(char(std::lround(127.5+110*std::sin(phase))));
 }
 std::vector<paging::FlexPage> pages;
 QString error;
 paging::FlexBridge bridge(iqRate,[&](auto page){pages.push_back(page);},[&](QString why){error=why;});
 if(!bridge.start()){std::cerr<<error.toStdString()<<std::endl;return 1;}
 for(qsizetype pos=0;pos<iq.size();pos+=65536)bridge.submit(iq.mid(pos,65536));
 bridge.stop();
 if(!error.isEmpty()||pages.size()!=1){std::cerr<<"pages="<<pages.size()<<" "<<error.toStdString()<<std::endl;return 1;}
 const auto& page=pages.front();
 if(page.address!=123456||page.baud!=1600||page.levels!=2||page.type!="alphanumeric"||page.text!="POCSAG SDR PRO FLEX TEST")return 1;
 std::cout<<"PASS synthetic FLEX CU8 through demodulator, external decoder and parser"<<std::endl;
 return 0;
}