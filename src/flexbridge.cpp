#include "flexbridge.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QJsonParseError>
#include <QtGlobal>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <numbers>

namespace paging {
FlexBridge::FlexBridge(double iqRate,PageHandler onPage,ErrorHandler onError):
 iqRate_(iqRate),onPage_(std::move(onPage)),onError_(std::move(onError)){}
FlexBridge::~FlexBridge(){stop();}

bool FlexBridge::start(){
 if(started_)return true;
 if(iqRate_<48000||iqRate_>20000000){onError_("FLEX : taux IQ hors limites");return false;}
 const auto path=QDir(QCoreApplication::applicationDirPath()).filePath(
#ifdef Q_OS_WIN
 "multimon-ng.exe"
#else
 "multimon-ng"
#endif
 );
 if(!QFileInfo(path).isFile()){
  onError_("FLEX indisponible : moteur multimon-ng FLEX_NEXT absent du dossier de l'application ; POCSAG reste opérationnel.");
  return false;
 }
 started_=true;
 thread_=std::thread([this]{run();});
 return true;
}
void FlexBridge::submit(const QByteArray& cu8){
 if(!started_||cu8.isEmpty())return;
 std::lock_guard lock(mutex_);
 if(closing_)return;
 constexpr std::size_t maxQueue=4*1024*1024;
 if(queuedBytes_+std::size_t(cu8.size())>maxQueue){
  // Do not silently drop IQ while claiming continuous FLEX decoding.
  if(!overflowReported_){overflowReported_=true;onError_("FLEX : retard du décodeur, échantillons IQ perdus ; synchronisation à reconfirmer");}
  return;
 }
 queuedBytes_+=std::size_t(cu8.size());
 queue_.push_back(cu8);
 wake_.notify_one();
}
void FlexBridge::stop(){
 {
  std::lock_guard lock(mutex_);
  closing_=true;
 }
 wake_.notify_all();
 if(thread_.joinable())thread_.join();
 started_=false;
}
void FlexBridge::appendPcm(double normalized,QByteArray& pcm){
 const auto value=static_cast<int>(std::lround(std::clamp(normalized,-1.0,1.0)*30000.0));
 const auto u=static_cast<unsigned>(static_cast<std::uint16_t>(static_cast<std::int16_t>(value)));
 pcm.append(char(u&255));
 pcm.append(char((u>>8)&255));
}
void FlexBridge::demod48(const std::complex<double>& sample,QByteArray& pcm){
 double audio=0;
 if(have48_){
  const auto product=sample*std::conj(previous48_);
  const double phase=std::atan2(product.imag(),product.real());
  dc_+=0.0003*(phase-dc_);
  audio=std::clamp((phase-dc_)*48000.0/(2*std::numbers::pi*7500.0),-1.0,1.0);
 }
 previous48_=sample;
 if(!have48_){
  have48_=true;
  lastAudio48_=audio;
  appendPcm(audio,pcm);
  nextAudioTick_=48000;
  ++audio48Index_;
  return;
 }
 const auto n=audio48Index_;
 // Target 22,050 Hz sample k occurs at input time k*48000/22050.
 while(nextAudioTick_<=n*22050ULL){
  const double fraction=double(nextAudioTick_-(n-1)*22050ULL)/22050.0;
  appendPcm(lastAudio48_+(audio-lastAudio48_)*fraction,pcm);
  nextAudioTick_+=48000;
 }
 lastAudio48_=audio;
 ++audio48Index_;
}
void FlexBridge::processIq(const QByteArray& cu8,QByteArray& pcm){
 // Four low-pass poles before decimation suppress wideband IQ noise.
 // The 48k FM discriminator is resampled to multimon-ng's 22,050 Hz PCM input.
 const double alpha=1-std::exp(-2*std::numbers::pi*8500.0/iqRate_);
 const double step=48000.0/iqRate_;
 const auto* data=reinterpret_cast<const unsigned char*>(cu8.constData());
 for(qsizetype i=0;i+1<cu8.size();i+=2){
  const std::complex<double> sample((double(data[i])-127.5)/128.0,(double(data[i+1])-127.5)/128.0);
  lowpass_[0]+=alpha*(sample-lowpass_[0]);
  for(std::size_t p=1;p<lowpass_.size();++p)lowpass_[p]+=alpha*(lowpass_[p-1]-lowpass_[p]);
  inputPhase_+=step;
  if(inputPhase_>=1){
   inputPhase_-=1;
   demod48(lowpass_.back(),pcm);
  }
 }
}
void FlexBridge::parseLine(const QByteArray& line){
 QJsonParseError error{};
 auto doc=QJsonDocument::fromJson(line,&error);
 if(error.error!=QJsonParseError::NoError||!doc.isObject())return;
 const auto o=doc.object();
 if(!o.contains("capcode")||!o.contains("msg_type")||!o.contains("baud")||!o.contains("level"))return;
 if(o.contains("k_ok")&&!o.value("k_ok").toBool())return;
 if(o.contains("sig_ok")&&!o.value("sig_ok").toBool())return;
 const auto address=o.value("capcode").toDouble(-1);
 if(!std::isfinite(address)||address<0||address>4297068542.0||std::floor(address)!=address)return;
 const int baud=o.value("baud").toInt(),levels=o.value("level").toInt();
 if((baud!=1600&&baud!=3200)||(levels!=2&&levels!=4))return;
 const QString type=o.value("msg_type").toString();
 const QString text=o.value("message").toString();
 if(text.trimmed().isEmpty()&&type!="tone_only")return;
 if(type=="unknown"||type=="instruction"||type=="short_msg")return;
 onPage_({quint64(address),baud,levels,type,text});
}
void FlexBridge::parseOutput(const QByteArray& bytes){
 outputLines_+=bytes;
 if(outputLines_.size()>128*1024){outputLines_.clear();onError_("FLEX : sortie du décodeur trop longue, ligne ignorée");return;}
 while(true){
  const auto newline=outputLines_.indexOf('\n');
  if(newline<0)break;
  const auto line=outputLines_.left(newline).trimmed();
  outputLines_.remove(0,newline+1);
  if(!line.isEmpty())parseLine(line);
 }
}
void FlexBridge::run(){
 QProcess process;
 const auto path=QDir(QCoreApplication::applicationDirPath()).filePath(
#ifdef Q_OS_WIN
 "multimon-ng.exe"
#else
 "multimon-ng"
#endif
 );
 process.setProcessChannelMode(QProcess::SeparateChannels);
 process.start(path,{"-c","-a","FLEX_NEXT","--json","-q","-v","0","-t","raw","-"},QIODevice::ReadWrite);
 if(!process.waitForStarted(5000)){
  onError_("FLEX : lancement du moteur impossible : "+process.errorString());
  return;
 }
 while(true){
  QByteArray iq;
  {
   std::unique_lock lock(mutex_);
   if(queue_.empty()&&!closing_)wake_.wait_for(lock,std::chrono::milliseconds(15));
   if(queue_.empty()&&closing_)break;
   if(!queue_.empty()){
    iq=std::move(queue_.front());
    queuedBytes_-=std::size_t(iq.size());
    queue_.pop_front();
    if(queuedBytes_<1024*1024)overflowReported_=false;
   }
  }
  if(!iq.isEmpty()){
   QByteArray pcm;
   pcm.reserve(int(double(iq.size())/2.0*22050.0/iqRate_*2.0)+64);
   processIq(iq,pcm);
   if(!pcm.isEmpty()&&process.state()==QProcess::Running){
    if(process.write(pcm)!=pcm.size()){
     onError_("FLEX : écriture PCM impossible");
     break;
    }
    if(process.bytesToWrite()>2*1024*1024){
     onError_("FLEX : moteur externe trop lent, abandon du flux sans faux messages");
     break;
    }
    process.waitForBytesWritten(1);
   }
  }
  process.waitForReadyRead(1);
  parseOutput(process.readAllStandardOutput());
  if(process.state()!=QProcess::Running){
   onError_("FLEX : moteur externe arrêté : "+process.errorString());
   break;
  }
 }
 process.closeWriteChannel();
 process.waitForFinished(1500);
 parseOutput(process.readAllStandardOutput());
 if(process.state()!=QProcess::NotRunning){process.kill();process.waitForFinished(1000);}
}
} // namespace paging
