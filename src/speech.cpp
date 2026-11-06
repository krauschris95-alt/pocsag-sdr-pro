#include "announcement.h"
#include "speech.h"
#include <QMutexLocker>
#include <stdexcept>
#include <QElapsedTimer>
#include <QXmlStreamWriter>
#include <QBuffer>
#include <QDir>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <sapi.h>
#include <sphelper.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
namespace {
void check(HRESULT hr,const char* action){if(FAILED(hr))throw std::runtime_error(std::string(action)+" (HRESULT "+std::to_string(unsigned(hr))+")");}
struct Com {HRESULT hr=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);~Com(){if(SUCCEEDED(hr))CoUninitialize();}};
QVector<SpeechChoice> choices(const wchar_t* category){
 Com com;QVector<SpeechChoice> result;if(FAILED(com.hr))return result;
 IEnumSpObjectTokens* tokens=nullptr;if(FAILED(SpEnumTokens(category,nullptr,nullptr,&tokens)))return result;
 ISpObjectToken* token=nullptr;while(tokens->Next(1,&token,nullptr)==S_OK){wchar_t *id=nullptr,*description=nullptr;token->GetId(&id);SpGetDescription(token,&description);bool french=false;ISpDataKey* attrs=nullptr;if(SUCCEEDED(token->OpenKey(L"Attributes",&attrs))){wchar_t* language=nullptr;if(SUCCEEDED(attrs->GetStringValue(L"Language",&language))){for(const auto& code:QString::fromWCharArray(language).split(';')){bool ok;auto locale=code.toUInt(&ok,16);if(ok&&(locale&0x3ff)==LANG_FRENCH)french=true;}CoTaskMemFree(language);}attrs->Release();}result.append({QString::fromWCharArray(id?id:L""),QString::fromWCharArray(description?description:L""),french});CoTaskMemFree(id);CoTaskMemFree(description);token->Release();}tokens->Release();return result;
}
struct AudioEndpoint{QString id;bool muted=false;float volume=0;};
AudioEndpoint defaultEndpoint(){AudioEndpoint out;IMMDeviceEnumerator* enumerator=nullptr;IMMDevice* device=nullptr;IAudioEndpointVolume* volume=nullptr;auto hr=CoCreateInstance(CLSID_MMDeviceEnumerator,nullptr,CLSCTX_ALL,IID_IMMDeviceEnumerator,reinterpret_cast<void**>(&enumerator));if(SUCCEEDED(hr)){if(SUCCEEDED(enumerator->GetDefaultAudioEndpoint(eRender,eConsole,&device))){wchar_t* id=nullptr;if(SUCCEEDED(device->GetId(&id))){out.id=QString::fromWCharArray(id);CoTaskMemFree(id);}if(SUCCEEDED(device->Activate(IID_IAudioEndpointVolume,CLSCTX_ALL,nullptr,reinterpret_cast<void**>(&volume)))){BOOL muted=false;volume->GetMute(&muted);volume->GetMasterVolumeLevelScalar(&out.volume);out.muted=muted;volume->Release();}device->Release();}enumerator->Release();}return out;}
class SapiBackend:public SpeechBackend {
 Com com_;ISpVoice* voice_=nullptr;QString diagnostic_;AudioEndpoint endpoint_;bool defaultOutput_=true,synthesizing_=false;
public:
 ~SapiBackend(){if(voice_)voice_->Release();}
 void configure(const SpeechSettings& settings)override{
  defaultOutput_=settings.output.isEmpty();endpoint_=defaultEndpoint();diagnostic_.clear();check(com_.hr,"Initialisation COM voix");if(voice_){voice_->Release();voice_=nullptr;}if(!voice_)check(CoCreateInstance(CLSID_SpVoice,nullptr,CLSCTX_INPROC_SERVER,IID_ISpVoice,reinterpret_cast<void**>(&voice_)),"Création voix Windows");
  auto token=[&](const QString& id,bool output){if(id.isEmpty()){if(output)check(voice_->SetOutput(nullptr,TRUE),"Sortie audio par défaut");else check(voice_->SetVoice(nullptr),"Voix par défaut");return;}ISpObjectToken* t=nullptr;check(CoCreateInstance(CLSID_SpObjectToken,nullptr,CLSCTX_INPROC_SERVER,IID_ISpObjectToken,reinterpret_cast<void**>(&t)),"Création jeton voix");auto idResult=t->SetId(nullptr,reinterpret_cast<const wchar_t*>(id.utf16()),FALSE);if(FAILED(idResult)){t->Release();check(idResult,"Voix ou sortie indisponible");}auto hr=output?voice_->SetOutput(t,TRUE):voice_->SetVoice(t);t->Release();check(hr,"Sélection voix ou sortie");};
  auto chosen=settings.voice;auto available=choices(SPCAT_VOICES);bool found=false;for(auto v:available)if(v.id==chosen)found=true;if(!found){chosen.clear();for(auto v:available)if(v.french){chosen=v.id;break;}}token(chosen,false);try{token(settings.output,true);}catch(...){defaultOutput_=true;token("",true);diagnostic_="Sortie configurée indisponible ; sortie Windows par défaut réinitialisée. ";}diagnostic_+="Voix="+(chosen.isEmpty()?QString("Windows par défaut"):chosen.section('\\',-1))+" ; sortie="+(defaultOutput_?QString("Windows par défaut (réouverte pour cette annonce)"):QString("sortie SAPI configurée"));
  diagnostic_+=" ; périphérique="+(endpoint_.id.isEmpty()?QString("ABSENT"):endpoint_.id)+" ; muet="+QString::number(endpoint_.muted)+" ; volume Windows="+QString::number(int(endpoint_.volume*100));check(voice_->SetVolume(USHORT(settings.volume)),"Volume vocal");check(voice_->SetRate(settings.rate),"Vitesse vocale");
 }
 void speak(const QString& text,const std::atomic_bool& cancel)override{
  QString xml;QXmlStreamWriter writer(&xml);writer.writeStartElement("speak");writer.writeCharacters(text);writer.writeEndElement();check(voice_->Speak(reinterpret_cast<const wchar_t*>(xml.utf16()),SPF_ASYNC|SPF_IS_XML,nullptr),"Lecture vocale");
  QElapsedTimer deadline,deviceCheck;deadline.start();deviceCheck.start();
  while(!cancel){HRESULT hr=voice_->WaitUntilDone(50);if(hr==S_OK)return;if(hr!=S_FALSE)check(hr,"Attente voix Windows");if(defaultOutput_&&!synthesizing_&&deviceCheck.elapsed()>500){deviceCheck.restart();if(defaultEndpoint().id!=endpoint_.id){voice_->Speak(nullptr,SPF_ASYNC|SPF_PURGEBEFORESPEAK,nullptr);throw std::runtime_error("Périphérique audio par défaut changé ou déconnecté ; réinitialisation requise");}}if(deadline.elapsed()>600000)throw std::runtime_error("Délai vocal dépassé ; sortie à réinitialiser");}
  check(voice_->Speak(nullptr,SPF_ASYNC|SPF_PURGEBEFORESPEAK,nullptr),"Arrêt voix Windows");
 }
 QString diagnostic()const override{return diagnostic_;}
 void synthesize(const QString& text,const QString& path,const std::atomic_bool& cancel)override{
  ISpStream* stream=nullptr;check(CoCreateInstance(CLSID_SpStream,nullptr,CLSCTX_INPROC_SERVER,IID_ISpStream,reinterpret_cast<void**>(&stream)),"Création flux WAV");
  try{GUID waveFormatId{};check(CLSIDFromString(L"{C31ADBAE-527F-4FF5-A230-F62BB61FF70C}",&waveFormatId),"Format PCM Windows");WAVEFORMATEX format{};format.wFormatTag=WAVE_FORMAT_PCM;format.nChannels=1;format.nSamplesPerSec=22050;format.wBitsPerSample=16;format.nBlockAlign=2;format.nAvgBytesPerSec=44100;auto nativePath=QDir::toNativeSeparators(path);check(stream->BindToFile(reinterpret_cast<const wchar_t*>(nativePath.utf16()),SPFM_CREATE_ALWAYS,&waveFormatId,&format,0),"Création fichier WAV");check(voice_->SetOutput(stream,TRUE),"Sortie WAV");synthesizing_=true;speak(text,cancel);synthesizing_=false;check(voice_->SetOutput(nullptr,TRUE),"Fermeture sortie WAV");check(stream->Close(),"Finalisation WAV");stream->Release();}catch(...){synthesizing_=false;voice_->SetOutput(nullptr,TRUE);stream->Close();stream->Release();throw;}
 }

};
}
#endif
SpeechService::SpeechService(std::unique_ptr<SpeechBackend> backend,QObject* parent):QThread(parent),backend_(std::move(backend)){}
SpeechService::~SpeechService(){shutdown();}
QVector<SpeechChoice> SpeechService::voices(){
#ifdef _WIN32
 return choices(SPCAT_VOICES);
#else
 return {};
#endif
}
QVector<SpeechChoice> SpeechService::outputs(){
#ifdef _WIN32
 return choices(SPCAT_AUDIOOUT);
#else
 return {};
#endif
}
QString SpeechService::utterance(const SpeechJob& j){return preparedAnnouncement(j.alias,j.text);}
void SpeechService::configure(SpeechSettings s){QMutexLocker lock(&mutex_);s.volume=qBound(0,s.volume,100);s.rate=qBound(-10,s.rate,10);s.limit=qBound(1,s.limit,10000);settings_=s;if(!s.enabled){jobs_.clear();cancel_=true;}ready_.wakeAll();}
bool SpeechService::enqueue(SpeechJob job){
 QMutexLocker lock(&mutex_);if(!settings_.enabled||closing_||job.text.trimmed().isEmpty()||seen_.contains(job.eventId)){emit diagnostic("Voix refusée : coupure="+QString::number(!settings_.enabled)+" ; doublon="+QString::number(seen_.contains(job.eventId))+" ; arrêt="+QString::number(closing_));return false;}
 if(jobs_.size()>=size_t(settings_.limit)){if(!settings_.dropOldest){emit skipped("File vocale pleine : nouveau message non lu (historique conservé)");return false;}jobs_.pop_front();emit skipped("File vocale pleine : plus ancien message en attente non lu (historique conservé)");}
 seen_.insert(job.eventId);if(seen_.size()>20000)seen_.clear();emit diagnostic("Voix ajoutée à la file : événement="+job.eventId);jobs_.push_back(std::move(job));ready_.wakeOne();return true;
}
void SpeechService::stopSpeaking(){QMutexLocker lock(&mutex_);jobs_.clear();cancel_=true;ready_.wakeAll();}
void SpeechService::shutdown(){{QMutexLocker lock(&mutex_);closing_=true;jobs_.clear();cancel_=true;ready_.wakeAll();}wait();}
int SpeechService::pending(){QMutexLocker lock(&mutex_);return int(jobs_.size());}
void SpeechService::run(){
#ifdef _WIN32
 if(!backend_)backend_=std::make_unique<SapiBackend>();
#endif
 for(;;){SpeechJob job;SpeechSettings settings;{QMutexLocker lock(&mutex_);while(!closing_&&jobs_.empty())ready_.wait(&mutex_);if(closing_)break;job=std::move(jobs_.front());jobs_.pop_front();settings=settings_;cancel_=false;}
  for(int attempt=0;attempt<2&&!cancel_;++attempt){try{if(!backend_)throw std::runtime_error("Synthèse vocale Windows non disponible");backend_->configure(settings);emit diagnostic(backend_->diagnostic());emit diagnostic("Lecture commencée : événement="+job.eventId);emit speaking(job.eventId);backend_->speak(utterance(job),cancel_);if(!cancel_){emit diagnostic("Lecture terminée : événement="+job.eventId);emit completed(job.eventId);}else emit diagnostic("Lecture annulée : événement="+job.eventId);break;}catch(const std::exception& e){emit problem(QString::fromUtf8(e.what()));if(attempt==0&&!cancel_)emit diagnostic("Réinitialisation contrôlée de la voix : événement="+job.eventId);}}
 }backend_.reset(); // COM voice is destroyed on the same thread that created it.
}


std::unique_ptr<SpeechBackend> makeWindowsSpeechBackend(){
#ifdef _WIN32
 return std::make_unique<SapiBackend>();
#else
 return {};
#endif
}
