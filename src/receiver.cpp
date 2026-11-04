#include "receiver.h"
#include "dsp.h"
#include <QLibrary>
#include <QFile>
#include <stdexcept>
namespace {
template<class F> F symbol(QLibrary& l,const char* n){auto p=l.resolve(n);if(!p)throw std::runtime_error(std::string("Symbole pilote manquant : ")+n);return reinterpret_cast<F>(p);}
}
QStringList Receiver::devices(){
 QLibrary l("rtlsdr");QStringList result;
 if(!l.load())return result;
 try{auto count=symbol<uint32_t(*)()>(l,"rtlsdr_get_device_count");auto name=symbol<const char*(*)(uint32_t)>(l,"rtlsdr_get_device_name");auto serial=symbol<int(*)(uint32_t,char*,char*,char*)>(l,"rtlsdr_get_device_usb_strings");
 for(uint32_t i=0;i<count();++i){char m[256]{},p[256]{},s[256]{};serial(i,m,p,s);result<<QString("%1 — %2").arg(name(i),s);}
 }catch(...){ }return result;
}
void Receiver::run(){
 stopping=false;
 pocsag::Demodulator dsp([this](pocsag::Message m){emit decoded(m.ric,m.function,m.baud,QString::fromStdString(m.type),QString::fromStdString(m.text),m.corrected,config.frequency,config.file.isEmpty()?QString("RTL-SDR index %1").arg(config.index):config.file);},config.baud,config.rate);
 int frame=0;auto consume=[&](const QByteArray& b){dsp.cu8(std::span(reinterpret_cast<const unsigned char*>(b.constData()),size_t(b.size())));if((frame++%8)==0)emit samples(b);};
 if(!config.file.isEmpty()){
  QFile f(config.file);if(!f.open(QIODevice::ReadOnly)){emit problem(f.errorString());return;}
  emit state("Lecture IQ CU8 hors ligne");
  while(!stopping&&!f.atEnd()){auto b=f.read(16384);if(b.size()%2){emit problem("Fichier IQ tronqué : paire I/Q incomplète");return;}consume(b);msleep(5);}
  dsp.finish();emit state("Lecture IQ terminée");return;
 }
 QLibrary l("rtlsdr");void* dev=nullptr;int(*close)(void*)=nullptr;
 try{
  if(!l.load())throw std::runtime_error("rtlsdr.dll absent. Installer le pilote WinUSB et les bibliothèques RTL-SDR Blog.");
  auto open=symbol<int(*)(void**,uint32_t)>(l,"rtlsdr_open");close=symbol<int(*)(void*)>(l,"rtlsdr_close");
  auto freq=symbol<int(*)(void*,uint32_t)>(l,"rtlsdr_set_center_freq");auto rate=symbol<int(*)(void*,uint32_t)>(l,"rtlsdr_set_sample_rate");
  auto ppm=symbol<int(*)(void*,int)>(l,"rtlsdr_set_freq_correction");auto mode=symbol<int(*)(void*,int)>(l,"rtlsdr_set_tuner_gain_mode");auto gain=symbol<int(*)(void*,int)>(l,"rtlsdr_set_tuner_gain");
  auto reset=symbol<int(*)(void*)>(l,"rtlsdr_reset_buffer");auto read=symbol<int(*)(void*,void*,int,int*)>(l,"rtlsdr_read_sync");
  if(open(&dev,config.index)<0)throw std::runtime_error("Impossible d'ouvrir le RTL-SDR (pilote ou appareil occupé)");
  auto check=[](int result,const char* name){if(result<0)throw std::runtime_error(std::string(name)+" refusé (code "+std::to_string(result)+")");};
  check(rate(dev,config.rate),"Taux IQ");check(freq(dev,config.frequency),"Fréquence");
  int ppmResult=ppm(dev,config.ppm);if(ppmResult!=-2)check(ppmResult,"Correction PPM");
  check(mode(dev,1),"Gain manuel");
  auto gains=symbol<int(*)(void*,int*)>(l,"rtlsdr_get_tuner_gains");int nGains=gains(dev,nullptr);if(nGains<=0)throw std::runtime_error("Liste des gains indisponible");std::vector<int> supported(size_t(nGains),0);gains(dev,supported.data());int nearest=*std::min_element(supported.begin(),supported.end(),[&](int a,int b){return abs(a-config.gain)<abs(b-config.gain);});
  check(gain(dev,nearest),"Gain tuner");check(reset(dev),"Réinitialisation du tampon");
  emit state("Réception RTL-SDR en cours");
  while(!stopping){QByteArray b(16384,0);int n=0;if(read(dev,b.data(),b.size(),&n)<0||n<=0)throw std::runtime_error("Erreur de lecture USB — réception interrompue");b.resize(n);consume(b);}
 }catch(const std::exception& e){emit problem(QString::fromUtf8(e.what()));}
 if(dev&&close)close(dev);
 dsp.finish();
}
