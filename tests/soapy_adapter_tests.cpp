#include "soapyradio.h"
#include "dsp.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QLibrary>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
int main(int argc,char** argv){QCoreApplication app(argc,argv);QTemporaryDir temp;QFile zipped(QString::fromUtf8(FIXTURE_DIRECTORY)+"/1200-alpha.iqz");if(!zipped.open(QIODevice::ReadOnly))return 2;auto iq=qUncompress(zipped.readAll());QFile file(temp.filePath("reference.cu8"));if(!file.open(QIODevice::WriteOnly))return 2;file.write(iq);file.close();QString path=QCoreApplication::applicationDirPath()+"/mock_soapy.dll";QLibrary dll(path);if(!dll.load())return 2;auto set=reinterpret_cast<void(*)(const char*)>(dll.resolve("mock_fixture"));auto fault=reinterpret_cast<void(*)(int)>(dll.resolve("mock_fault"));if(!set||!fault)return 2;set(file.fileName().toUtf8().constData());qputenv("POCSAG_TEST_SOAPY_LIBRARY",path.toUtf8());QString diagnostic;auto devices=SoapyRadio::devices(&diagnostic);int errors=devices.size()!=4;
for(auto d:devices){auto c=SoapyRadio::capabilities(d.key);if(!c.error.isEmpty()||!c.rates.contains(240000)||!c.agc)++errors;RadioConfig config;config.deviceKey=d.key;config.rate=240000;config.frequency=169626000;config.automaticGain=true;std::atomic_bool stop=false;int frames=0,tuned=0;qsizetype bytes=0;pocsag::Demodulator demod([&](pocsag::Message m){if(m.text=="REFERENCE 012 FUNCTION TEST")++frames;});try{SoapyRadio::receive(config,stop,[&](QByteArray b){demod.cu8(std::span(reinterpret_cast<const unsigned char*>(b.constData()),size_t(b.size())));bytes+=b.size();if(bytes>=iq.size())stop=true;},[&](quint32 f,quint32 r){if(f==169626000&&r==240000)++tuned;},[](QString){});}catch(...){++errors;}demod.finish();if(frames!=1||tuned!=1)++errors;
for(int mode:{1,2,3,4,5,6}){fault(mode);stop=false;bool rejected=false;try{SoapyRadio::receive(config,stop,[](QByteArray){},[](quint32,quint32){},[](QString){});}catch(const std::exception&){rejected=true;}if(!rejected)++errors;fault(0);}}
std::cout<<"Soapy simulated API: 4 driver labels, reference decoding, readback mismatch, transport failure, nonfinite/oversize, open failure, incompatible ABI; errors="<<errors<<". No vendor hardware qualification.\n";return errors?1:0;}
