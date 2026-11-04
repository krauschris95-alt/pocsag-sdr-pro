#include "dsp.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
int main(int argc,char** argv){QCoreApplication app(argc,argv);try{
 QString root=QString::fromUtf8(FIXTURE_DIRECTORY);QFile manifest(root+"/manifest.json");if(!manifest.open(QIODevice::ReadOnly))throw std::runtime_error("fixture manifest unavailable");
 int count=0;for(auto item:QJsonDocument::fromJson(manifest.readAll()).array()){
  auto entry=item.toObject();QFile file(root+"/"+entry["file"].toString());if(!file.open(QIODevice::ReadOnly))throw std::runtime_error("fixture unavailable");auto iq=qUncompress(file.readAll());if(iq.isEmpty())throw std::runtime_error("fixture corrupted");std::vector<pocsag::Message> messages;
  pocsag::Demodulator dsp([&](pocsag::Message m){m=pocsag::interpret(std::move(m),entry["mode"].toString().toStdString());if(m.type!="TONE"&&m.type!="DIAGNOSTIC")messages.push_back(m);});
  for(qsizetype i=0;i<iq.size();i+=16384)dsp.cu8(std::span(reinterpret_cast<const unsigned char*>(iq.constData()+i),size_t(std::min(qsizetype(16384),iq.size()-i))));dsp.finish();
  if(messages.size()!=1)throw std::runtime_error("independent fixture count mismatch");auto& m=messages[0];if(m.ric!=entry["ric"].toInt()||m.function!=entry["function"].toInt()||m.baud!=entry["baud"].toInt()||m.type!=entry["type"].toString().toStdString()||m.text!=entry["text"].toString().toStdString())throw std::runtime_error("independent fixture payload mismatch");++count;
 }std::cout<<count<<" independent IQ fixtures PASS\n";return count==6?0:1;
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
