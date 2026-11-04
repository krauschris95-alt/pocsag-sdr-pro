#include "speech.h"
#include <QCoreApplication>
#include <QTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
int main(int argc,char** argv){QCoreApplication app(argc,argv);if(argc<2)return 2;auto choices=SpeechService::voices();SpeechSettings settings;settings.enabled=true;settings.volume=65;QJsonArray voices;for(auto& choice:choices){voices.append(choice.name);if(choice.french&&settings.voice.isEmpty())settings.voice=choice.id;}SpeechService speech;QString error;bool completed=false;QObject::connect(&speech,&SpeechService::problem,&app,[&](QString value){error=value;app.quit();});QObject::connect(&speech,&SpeechService::completed,&app,[&]{completed=true;app.quit();});speech.configure(settings);speech.start();speech.enqueue({"voice-probe","POCSAG SDR Pro","Test de lecture vocale Windows. Le nom est annoncé avant le message."});QTimer::singleShot(20000,&app,&QCoreApplication::quit);app.exec();speech.shutdown();QFile report(QString::fromLocal8Bit(argv[1]));if(!report.open(QIODevice::WriteOnly))return 2;report.write(QJsonDocument(QJsonObject{{"voices",voices},{"french_selected",!settings.voice.isEmpty()},{"sapi_completed",completed},{"error",error}}).toJson());return completed?0:1;}
