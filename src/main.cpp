#include "window.h"
#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QSettings>
int main(int argc,char** argv){
 QApplication app(argc,argv);app.setOrganizationName("POCSAG-SDR-Pro");app.setApplicationName("POCSAG-SDR-Pro");auto args=app.arguments();
 int probe=args.indexOf("--probe-report");
 if(probe>=0&&probe+1<args.size()){
  Receiver receiver;int freqArg=args.indexOf("--frequency");if(freqArg>=0&&freqArg+1<args.size())receiver.config.frequency=quint32(args[freqArg+1].toDouble()*1e6);int seconds=5;int secArg=args.indexOf("--seconds");if(secArg>=0&&secArg+1<args.size())seconds=std::clamp(args[secArg+1].toInt(),1,60);QString error,state;quint64 bytes=0;int messages=0;auto devices=Receiver::devices();
  QObject::connect(&receiver,&Receiver::problem,&app,[&](QString s){error=s;});QObject::connect(&receiver,&Receiver::state,&app,[&](QString s){state=s;});
  QObject::connect(&receiver,&Receiver::samples,&app,[&](QByteArray b){bytes+=b.size();});QObject::connect(&receiver,&Receiver::decoded,&app,[&]{++messages;});
  QTimer::singleShot(seconds*1000,&app,[&]{receiver.stop();QFile f(args[probe+1]);if(!f.open(QIODevice::WriteOnly)){app.exit(2);return;}f.write(QJsonDocument(QJsonObject{{"devices",devices.join("; ")},{"frequency_hz",double(receiver.config.frequency)},{"sample_rate",double(receiver.config.rate)},{"duration_seconds",seconds},{"spectrum_bytes",double(bytes)},{"messages_decoded",messages},{"state",state},{"error",error},{"hardware_read_ok",bytes>0&&error.isEmpty()}}).toJson());app.exit(bytes>0&&error.isEmpty()?0:1);});receiver.start();return app.exec();
 }
 try{int freqArg=args.indexOf("--frequency");if(freqArg>=0&&freqArg+1<args.size())QSettings().setValue("frequency",args[freqArg+1].toDouble());Window w;w.show();int shot=args.indexOf("--screenshot");if(shot>=0&&shot+1<args.size())QTimer::singleShot(500,&w,[&]{bool ok=w.grab().save(args[shot+1]);app.exit(ok?0:2);});return app.exec();}catch(const std::exception& e){QMessageBox::critical(nullptr,"Démarrage impossible",QString::fromUtf8(e.what()));return 1;}
}
