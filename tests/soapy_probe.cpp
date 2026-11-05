#include "soapyradio.h"
#include "cpucaps.h"
#include <QCoreApplication>
#include <QTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
int main(int argc,char** argv){QCoreApplication app(argc,argv);auto args=app.arguments();if(args.size()<2)return 2;QString diagnostic;auto devices=SoapyRadio::devices(&diagnostic);QJsonArray names;QString chosen;for(auto d:devices){names.append(d.name);if(chosen.isEmpty()&&d.name.contains("Soapy:rtlsdr"))chosen=d.key;}QJsonObject report{{"devices",names},{"diagnostic",diagnostic},{"cpu",cpuCapabilities()}};
 if(args.contains("--hardware")&&!chosen.isEmpty()){auto caps=SoapyRadio::capabilities(chosen);report.insert("capabilities_error",caps.error);report.insert("agc_supported",caps.agc);report.insert("ppm_supported",caps.ppm);QJsonArray rates;for(double rate:caps.rates)rates.append(rate);report.insert("rates",rates);Receiver radio;radio.config.deviceKey=chosen;radio.config.label="RTL via SoapySDR";radio.config.frequency=169626000;radio.config.rate=250000;int seconds=args.contains("--soak")?300:60;int messages=0;quint64 bytes=0;QString error;bool green=false;QTimer timer;QObject::connect(&timer,&QTimer::timeout,[&]{green|=radio.hasLiveIq();});timer.start(100);QObject::connect(&radio,&Receiver::problem,[&](QString text){error=text;});QObject::connect(&radio,&Receiver::samples,[&](QByteArray iq){bytes+=iq.size();});QObject::connect(&radio,&Receiver::frame,[&](pocsag::Message m){if(m.type=="ALPHA"||m.type=="NUMERIC")++messages;});QTimer::singleShot(seconds*1000,&app,&QCoreApplication::quit);radio.start();app.exec();radio.stop();report.insert("duration_seconds",seconds);report.insert("texts",messages);report.insert("spectrum_bytes",double(bytes));report.insert("green_iq",green);report.insert("red_after_stop",!radio.hasLiveIq());report.insert("error",error);report.insert("hardware_read_ok",green&&bytes>0&&error.isEmpty());}
 QFile file(args[1]);if(!file.open(QIODevice::WriteOnly))return 2;file.write(QJsonDocument(report).toJson());return args.contains("--hardware")?report.value("hardware_read_ok").toBool()?0:1:0;
}
