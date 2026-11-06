#include "speech.h"
#include "announcement.h"
#include <QCoreApplication>
#include <QMutexLocker>
#include <QSemaphore>
#include <iostream>
struct State {QMutex mutex;QStringList said;QSemaphore began;std::atomic_bool release=false;std::atomic_int active=0,maximum=0;};
struct Mock:SpeechBackend {State& state;explicit Mock(State& s):state(s){}void configure(const SpeechSettings&)override{}void speak(const QString& value,const std::atomic_bool& cancel)override{int active=++state.active;state.maximum=qMax(int(state.maximum),active);{QMutexLocker l(&state.mutex);state.said<<value;}state.began.release();while(!state.release&&!cancel)QThread::msleep(2);--state.active;}};
void require(bool value){if(!value)throw std::runtime_error("speech assertion failed");}
void interventionCases(){
 const QString original="APPEL: 50263090555 MED, P002 - AGRESSION - BAGARRE, N5 0";
 require(preparedAnnouncement("départ ambulance R-MED1",original)=="départ ambulance R-MED1. P zéro zéro deux, agression, bagarre, N cinq, zéro");
 for(auto pair:QList<QPair<QString,QString>>{{"P004","P zéro zéro quatre"},{"P017","P zéro dix-sept"},{"P026","P zéro vingt-six"},{"P039","P zéro trente-neuf"},{"P069","P zéro soixante-neuf"}})require(interventionSpeech("APPEL 123 MED, "+pair.first+" SUITE 0007")==pair.second+" suite 0007");
 require(interventionSpeech("préfixe p026: TEST P039 FIN 09")=="P zéro vingt-six: test P zéro trente-neuf fin 09");
 for(auto value:QStringList{"XP002","P0029","P002A","_P002_","éP002","P0","P00","P0000","MESSAGE DE SERVICE"})require(interventionSpeech(value)==value);
 require(interventionSpeech("XP002 P0029 puis (p017), FIN")=="P zéro dix-sept), fin");
 require(preparedAnnouncement("CIP112","MESSAGE DE SERVICE")=="CIP112. MESSAGE DE SERVICE");
 require(preparedAnnouncement("Capcode 0000042","P002 TEST")=="Capcode 0000042. P zéro zéro deux test");
 require(interventionSpeech("APPEL 123 P039 12 victimes, 25 ans, 125 mètres, 39 personnes à 18h30. Rue Jean-Jaurès.")=="P zéro trente-neuf 12 victimes, 25 ans, 125 mètres, 39 personnes à dix-huit heures trente. rue jean-jaurès.");
 require(interventionSpeech("12 victimes. Téléphone 0471 12 34 56. Référence 0007.")=="12 victimes. Téléphone 0471 12 34 56. Référence 0007.");
 for(int n=0;n<100;++n){auto code=QString("P0%1").arg(n,2,10,QChar('0'));require(interventionSpeech(code)=="P zéro "+(n<10?QString("zéro "):QString())+frenchBelowHundred(n));}
 require(preparedAnnouncement("Alias.","Message final.")=="Alias. Message final.");
 require(original=="APPEL: 50263090555 MED, P002 - AGRESSION - BAGARRE, N5 0");
}
int main(int argc,char** argv){QCoreApplication app(argc,argv);try{interventionCases();State state;SpeechService service(std::make_unique<Mock>(state));SpeechSettings settings;settings.enabled=true;settings.limit=2;service.configure(settings);service.start();require(service.enqueue({"1","Ambulance Z3","Message complet."}));require(state.began.tryAcquire(1,2000));require(service.enqueue({"2","Alias 2","Deux"}));require(!service.enqueue({"2","Alias 2","Deux"}));require(service.enqueue({"3","Alias 3","Trois"}));require(!service.enqueue({"4","Alias 4","Quatre"}));settings.dropOldest=true;service.configure(settings);require(service.enqueue({"4","Alias 4","Quatre"}));state.release=true;require(state.began.tryAcquire(2,2000));service.stopSpeaking();settings.enabled=false;service.configure(settings);require(!service.enqueue({"5","Alias","Non lu"}));service.shutdown();require(state.maximum==1);require(state.said==QStringList{"Ambulance Z3. Message complet.","Alias 3. Trois","Alias 4. Quatre"});std::cout<<"PASS speech order, alias first, bounded queue policies, deduplication, disable, no overlap\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
