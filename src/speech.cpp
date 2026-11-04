#include "speech.h"
#include <QMutexLocker>
#include <stdexcept>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <sapi.h>
#include <sphelper.h>
namespace {
void check(HRESULT hr,const char* action){if(FAILED(hr))throw std::runtime_error(std::string(action)+" (HRESULT "+std::to_string(unsigned(hr))+")");}
struct Com {HRESULT hr=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);~Com(){if(SUCCEEDED(hr))CoUninitialize();}};
QVector<SpeechChoice> choices(const wchar_t* category){
 Com com;QVector<SpeechChoice> result;if(FAILED(com.hr))return result;
 IEnumSpObjectTokens* tokens=nullptr;if(FAILED(SpEnumTokens(category,nullptr,nullptr,&tokens)))return result;
 ISpObjectToken* token=nullptr;while(tokens->Next(1,&token,nullptr)==S_OK){wchar_t *id=nullptr,*description=nullptr;token->GetId(&id);SpGetDescription(token,&description);bool french=false;ISpDataKey* attrs=nullptr;if(SUCCEEDED(token->OpenKey(L"Attributes",&attrs))){wchar_t* language=nullptr;if(SUCCEEDED(attrs->GetStringValue(L"Language",&language))){for(const auto& code:QString::fromWCharArray(language).split(';')){bool ok;auto locale=code.toUInt(&ok,16);if(ok&&(locale&0x3ff)==LANG_FRENCH)french=true;}CoTaskMemFree(language);}attrs->Release();}result.append({QString::fromWCharArray(id?id:L""),QString::fromWCharArray(description?description:L""),french});CoTaskMemFree(id);CoTaskMemFree(description);token->Release();}tokens->Release();return result;
}
class SapiBackend:public SpeechBackend {
 Com com_;ISpVoice* voice_=nullptr;
public:
 ~SapiBackend(){if(voice_)voice_->Release();}
 void configure(const SpeechSettings& settings)override{
  check(com_.hr,"Initialisation COM voix");if(!voice_)check(CoCreateInstance(CLSID_SpVoice,nullptr,CLSCTX_INPROC_SERVER,IID_ISpVoice,reinterpret_cast<void**>(&voice_)),"Création voix Windows");
  auto token=[&](const QString& id,bool output){if(id.isEmpty()){if(output)check(voice_->SetOutput(nullptr,TRUE),"Sortie audio par défaut");else check(voice_->SetVoice(nullptr),"Voix par défaut");return;}ISpObjectToken* t=nullptr;check(CoCreateInstance(CLSID_SpObjectToken,nullptr,CLSCTX_INPROC_SERVER,IID_ISpObjectToken,reinterpret_cast<void**>(&t)),"Création jeton voix");auto idResult=t->SetId(nullptr,reinterpret_cast<const wchar_t*>(id.utf16()),FALSE);if(FAILED(idResult)){t->Release();check(idResult,"Voix ou sortie indisponible");}auto hr=output?voice_->SetOutput(t,TRUE):voice_->SetVoice(t);t->Release();check(hr,"Sélection voix ou sortie");};
  token(settings.voice,false);token(settings.output,true);check(voice_->SetVolume(USHORT(settings.volume)),"Volume vocal");check(voice_->SetRate(settings.rate),"Vitesse vocale");
 }
 void speak(const QString& text,const std::atomic_bool& cancel)override{
  check(voice_->Speak(reinterpret_cast<const wchar_t*>(text.utf16()),SPF_ASYNC|SPF_IS_NOT_XML,nullptr),"Lecture vocale");
  while(!cancel){HRESULT hr=voice_->WaitUntilDone(50);if(hr==S_OK)return;if(hr!=S_FALSE)check(hr,"Attente voix Windows");}
  check(voice_->Speak(nullptr,SPF_ASYNC|SPF_PURGEBEFORESPEAK,nullptr),"Arrêt voix Windows");
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
QString SpeechService::utterance(const SpeechJob& j){return j.alias.trimmed().isEmpty()?j.text:j.alias.trimmed()+". "+j.text;}
void SpeechService::configure(SpeechSettings s){QMutexLocker lock(&mutex_);s.volume=qBound(0,s.volume,100);s.rate=qBound(-10,s.rate,10);s.limit=qBound(1,s.limit,10000);settings_=s;if(!s.enabled){jobs_.clear();cancel_=true;}ready_.wakeAll();}
bool SpeechService::enqueue(SpeechJob job){
 QMutexLocker lock(&mutex_);if(!settings_.enabled||closing_||job.text.trimmed().isEmpty()||seen_.contains(job.eventId))return false;
 if(jobs_.size()>=size_t(settings_.limit)){if(!settings_.dropOldest){emit skipped("File vocale pleine : nouveau message non lu (historique conservé)");return false;}jobs_.pop_front();emit skipped("File vocale pleine : plus ancien message en attente non lu (historique conservé)");}
 seen_.insert(job.eventId);if(seen_.size()>20000)seen_.clear();jobs_.push_back(std::move(job));ready_.wakeOne();return true;
}
void SpeechService::stopSpeaking(){QMutexLocker lock(&mutex_);jobs_.clear();cancel_=true;ready_.wakeAll();}
void SpeechService::shutdown(){{QMutexLocker lock(&mutex_);closing_=true;jobs_.clear();cancel_=true;ready_.wakeAll();}wait();}
int SpeechService::pending(){QMutexLocker lock(&mutex_);return int(jobs_.size());}
void SpeechService::run(){
#ifdef _WIN32
 if(!backend_)backend_=std::make_unique<SapiBackend>();
#endif
 for(;;){SpeechJob job;SpeechSettings settings;{QMutexLocker lock(&mutex_);while(!closing_&&jobs_.empty())ready_.wait(&mutex_);if(closing_)break;job=std::move(jobs_.front());jobs_.pop_front();settings=settings_;cancel_=false;}
  try{if(!backend_)throw std::runtime_error("Synthèse vocale Windows non disponible");backend_->configure(settings);emit speaking(job.eventId);backend_->speak(utterance(job),cancel_);if(!cancel_)emit completed(job.eventId);}catch(const std::exception& e){emit problem(QString::fromUtf8(e.what()));}
 }backend_.reset(); // COM voice is destroyed on the same thread that created it.
}

