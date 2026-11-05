#include "receiver.h"
#include "dsp.h"
#include <QLibrary>
#include <QFile>
#include <QMutexLocker>
#include <functional>
#include <thread>
#include <chrono>
#include <stdexcept>
namespace {
template<class F> F sym(QLibrary& lib,const char* name){auto p=lib.resolve(name);if(!p)throw std::runtime_error(std::string("API pilote manquante : ")+name);return reinterpret_cast<F>(p);}
void check(int result,const char* operation){if(result<0)throw std::runtime_error(std::string(operation)+" : code "+std::to_string(result));}
struct HackList {char** serials;int* boards;int* indices;int count;void** devices;int usbCount;};
struct HackTransfer {void* device;unsigned char* buffer;int bufferLength,validLength;void *rxContext,*txContext;};
using Consumer=std::function<void(const QByteArray&)>;
struct Context {Consumer consume;std::atomic_bool* stop;std::atomic<long long> last{0};};
long long nowMs(){return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
void rtlCallback(unsigned char* data,uint32_t size,void* user){auto& c=*static_cast<Context*>(user);c.last=nowMs();if(!*c.stop)c.consume(QByteArray(reinterpret_cast<char*>(data),int(size)));}
int hackCallback(HackTransfer* t){auto& c=*static_cast<Context*>(t->rxContext);if(*c.stop)return -1;c.last=nowMs();QByteArray b(t->validLength,0);for(int i=0;i<t->validLength;++i)b[i]=char(t->buffer[i]^0x80);c.consume(b);return 0;}
}
QVector<RadioDevice> Receiver::deviceInfos(){
 QVector<RadioDevice> result;QLibrary rtl(qEnvironmentVariable("POCSAG_TEST_RTL_LIBRARY","rtlsdr"));if(rtl.load())try{auto count=sym<uint32_t(*)()>(rtl,"rtlsdr_get_device_count");auto name=sym<const char*(*)(uint32_t)>(rtl,"rtlsdr_get_device_name");auto strings=sym<int(*)(uint32_t,char*,char*,char*)>(rtl,"rtlsdr_get_device_usb_strings");for(uint32_t i=0;i<count();++i){char m[256]{},p[256]{},s[256]{};strings(i,m,p,s);QString id=QString::fromUtf8(s);result.push_back({QString("%1 %2 — %3").arg(QString::fromUtf8(name(i)),QString::fromUtf8(p),id),"rtl:"+(id.isEmpty()?QString::number(i):id)});}}catch(...){}
 QLibrary hack("libhackrf");if(hack.load())try{auto init=sym<int(*)()>(hack,"hackrf_init");auto exit=sym<int(*)()>(hack,"hackrf_exit");if(init()==0){auto list=sym<HackList*(*)()>(hack,"hackrf_device_list");auto free=sym<void(*)(HackList*)>(hack,"hackrf_device_list_free");auto devices=list();if(devices){for(int i=0;i<devices->count;++i){QString serial=QString::fromUtf8(devices->serials[i]?devices->serials[i]:"");if(!serial.isEmpty())result.push_back({"HackRF One — "+serial,"hackrf:"+serial});}free(devices);}exit();}}catch(...){}return result;
}
QStringList Receiver::devices(){QStringList names;for(auto d:deviceInfos())names<<d.name;return names;}
bool Receiver::hasLiveIq()const{auto last=lastLiveIq_.load();return isRunning()&&!stopping&&last>0&&nowMs()-last<1500;}
void Receiver::stop(){stopping=true;lastLiveIq_=0;{QMutexLocker lock(&deviceMutex_);if(activeDevice_&&cancelAsync_)cancelAsync_(activeDevice_);}wait();}
void Receiver::run(){
 QFile capture(config.capture);if(!config.capture.isEmpty()&&!capture.open(QIODevice::WriteOnly|QIODevice::NewOnly)){emit problem("Capture IQ privée : "+capture.errorString());return;}
 bool offline=!config.file.isEmpty();int retries=0;
 do {
  QString identity=config.deviceKey.isEmpty()?QString("RTL-SDR index %1").arg(config.index):config.deviceKey;
  pocsag::Demodulator dsp([&](pocsag::Message m){emit frame(m,config.frequency,identity,QDateTime::currentDateTimeUtc());emit decoded(m.ric,m.function,m.baud,QString::fromStdString(m.type),QString::fromStdString(m.text),m.corrected,config.frequency,identity);},config.baud,config.rate,config.channelBandwidth);
  int spectrumFrame=0;Consumer consume=[&](const QByteArray& b){if(!offline&&!b.isEmpty())lastLiveIq_=nowMs();if(capture.isOpen()&&capture.write(b)!=b.size()){stopping=true;emit problem("Erreur capture IQ");return;}double power=0;for(int i=0;i+1<b.size();i+=2){double re=(uchar(b[i])-127.5)/128,im=(uchar(b[i+1])-127.5)/128;power+=re*re+im*im;}double db=10*std::log10(qMax(1e-12,power/qMax(1,b.size()/2)));if(config.squelch>-160&&db<config.squelch)dsp.reset();else dsp.cu8(std::span(reinterpret_cast<const unsigned char*>(b.constData()),size_t(b.size())));if((spectrumFrame++%8)==0)emit samples(b);};
  if(offline){QFile file(config.file);if(!file.open(QIODevice::ReadOnly)){emit problem(file.errorString());return;}emit tuned(config.frequency,config.rate,config.bandwidth,config.file);emit state("Lecture IQ hors ligne");while(!stopping&&!file.atEnd()){auto b=file.read(16384);if(b.size()%2){emit problem("IQ tronqué");return;}consume(b);msleep(5);}dsp.finish();emit state("Lecture IQ terminée");return;}
  void* device=nullptr;int(*closeDevice)(void*)=nullptr;int(*exitHack)()=nullptr;int(*stopHack)(void*)=nullptr;QLibrary library(config.deviceKey.startsWith("hackrf:")?"libhackrf":qEnvironmentVariable("POCSAG_TEST_RTL_LIBRARY","rtlsdr"));
  try {
   if(!library.load())throw std::runtime_error((library.fileName()+" : "+library.errorString()).toStdString());
   Context context{consume,&stopping};context.last=nowMs();
   if(config.deviceKey.startsWith("hackrf:")){
    if(config.rate<2000000||config.rate>20000000)throw std::runtime_error("HackRF : taux requis de 2 à 20 Méch/s");if(config.automaticGain)throw std::runtime_error("HackRF : gain automatique non disponible");
    check(sym<int(*)()>(library,"hackrf_init")(),"Initialisation HackRF");exitHack=sym<int(*)()>(library,"hackrf_exit");closeDevice=sym<int(*)(void*)>(library,"hackrf_close");check(sym<int(*)(const char*,void**)>(library,"hackrf_open_by_serial")(config.deviceKey.mid(7).toUtf8().constData(),&device),"Ouverture HackRF");
    check(sym<int(*)(void*,double)>(library,"hackrf_set_sample_rate")(device,config.rate),"Taux HackRF");check(sym<int(*)(void*,uint64_t)>(library,"hackrf_set_freq")(device,uint64_t(std::llround(config.frequency*(1+config.ppm/1e6)))),"Fréquence HackRF corrigée PPM");
    check(sym<int(*)(void*,uint32_t)>(library,"hackrf_set_lna_gain")(device,uint32_t(qBound(0,config.gain/80*8,40))),"LNA HackRF");check(sym<int(*)(void*,uint32_t)>(library,"hackrf_set_vga_gain")(device,uint32_t(qBound(0,config.vgaGain/2*2,62))),"VGA HackRF");
    quint32 bandwidth=config.bandwidth?config.bandwidth:1750000;bandwidth=sym<uint32_t(*)(uint32_t)>(library,"hackrf_compute_baseband_filter_bw")(bandwidth);if(bandwidth>config.rate)throw std::runtime_error("Bande passante HackRF supérieure au taux IQ");check(sym<int(*)(void*,uint32_t)>(library,"hackrf_set_baseband_filter_bandwidth")(device,bandwidth),"Filtre HackRF");stopHack=sym<int(*)(void*)>(library,"hackrf_stop_rx");check(sym<int(*)(void*,int(*)(HackTransfer*),void*)>(library,"hackrf_start_rx")(device,hackCallback,&context),"Réception HackRF");emit tuned(config.frequency,config.rate,bandwidth,identity);emit state("Réception HackRF en cours");auto streaming=sym<int(*)(void*)>(library,"hackrf_is_streaming");while(!stopping&&streaming(device)>0&&nowMs()-context.last<3000)msleep(100);stopHack(device);stopHack=nullptr;if(!stopping)throw std::runtime_error("HackRF : flux interrompu ou USB déconnecté");
   }else{
    auto count=sym<uint32_t(*)()>(library,"rtlsdr_get_device_count");auto strings=sym<int(*)(uint32_t,char*,char*,char*)>(library,"rtlsdr_get_device_usb_strings");int index=config.index;if(!config.deviceKey.isEmpty()){index=-1;for(uint32_t i=0;i<count();++i){char m[256]{},p[256]{},s[256]{};strings(i,m,p,s);if(config.deviceKey=="rtl:"+QString::fromUtf8(s)){index=int(i);break;}}}if(index<0)throw std::runtime_error("RTL-SDR sélectionné absent ; attente du même numéro de série");
    closeDevice=sym<int(*)(void*)>(library,"rtlsdr_close");check(sym<int(*)(void**,uint32_t)>(library,"rtlsdr_open")(&device,index),"Ouverture RTL-SDR");check(sym<int(*)(void*,uint32_t)>(library,"rtlsdr_set_sample_rate")(device,config.rate),"Taux IQ");check(sym<int(*)(void*,uint32_t)>(library,"rtlsdr_set_center_freq")(device,config.frequency),"Fréquence");auto ppm=sym<int(*)(void*,int)>(library,"rtlsdr_set_freq_correction")(device,config.ppm);if(ppm!=-2)check(ppm,"PPM");check(sym<int(*)(void*,int)>(library,"rtlsdr_set_tuner_gain_mode")(device,config.automaticGain?0:1),"Gain automatique/manuel");if(!config.automaticGain){auto gains=sym<int(*)(void*,int*)>(library,"rtlsdr_get_tuner_gains");int n=gains(device,nullptr);if(n<=0)throw std::runtime_error("Gains RTL indisponibles");std::vector<int> values(size_t(n),0);gains(device,values.data());int nearest=*std::min_element(values.begin(),values.end(),[&](int a,int b){return abs(a-config.gain)<abs(b-config.gain);});check(sym<int(*)(void*,int)>(library,"rtlsdr_set_tuner_gain")(device,nearest),"Gain tuner");}
    if(config.bandwidth)check(sym<int(*)(void*,uint32_t)>(library,"rtlsdr_set_tuner_bandwidth")(device,config.bandwidth),"Bande passante tuner");check(sym<int(*)(void*)>(library,"rtlsdr_reset_buffer")(device),"Tampon RTL");auto cancel=sym<int(*)(void*)>(library,"rtlsdr_cancel_async");{QMutexLocker lock(&deviceMutex_);activeDevice_=device;cancelAsync_=cancel;}
    emit tuned(config.frequency,config.rate,config.channelBandwidth,identity);emit state(retries?"RTL-SDR reconnecté — réception reprise":"Réception RTL-SDR en cours");auto readAsync=sym<int(*)(void*,void(*)(unsigned char*,uint32_t,void*),void*,uint32_t,uint32_t)>(library,"rtlsdr_read_async");std::atomic_bool finished=false,stalled=false;std::thread watchdog([&]{while(!finished){if(stopping||nowMs()-context.last>3000){if(!stopping)stalled=true;cancel(device);}std::this_thread::sleep_for(std::chrono::milliseconds(100));}});
    int result=readAsync(device,rtlCallback,&context,0,16384);finished=true;watchdog.join();{QMutexLocker lock(&deviceMutex_);activeDevice_=nullptr;cancelAsync_=nullptr;}if(!stopping){if(stalled)throw std::runtime_error("RTL-SDR : aucun échantillon depuis 3 secondes");check(result,"Lecture USB asynchrone");throw std::runtime_error("Flux RTL interrompu");}
   }
  }catch(const std::exception& error){lastLiveIq_=0;if(!stopping)emit problem(QString::fromUtf8(error.what()));}
  {QMutexLocker lock(&deviceMutex_);activeDevice_=nullptr;cancelAsync_=nullptr;}if(device&&stopHack)stopHack(device);if(device&&closeDevice)closeDevice(device);if(exitHack)exitHack();dsp.finish();
  lastLiveIq_=0;if(stopping||!config.reconnect)break;++retries;emit state("Reconnexion SDR dans 3 secondes…");for(int i=0;i<30&&!stopping;++i)msleep(100);
 }while(!stopping);
}
