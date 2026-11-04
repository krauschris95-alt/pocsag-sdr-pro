#pragma once
#include <QThread>
#include <QStringList>
#include <atomic>
#include <QDateTime>
#include <QMetaType>
#include "decoder.h"
Q_DECLARE_METATYPE(pocsag::Message)
struct RadioConfig { quint32 frequency=169626000, rate=240000; int gain=420,ppm=0,index=0; QString file; int baud=0; QString capture; };
class Receiver:public QThread {
 Q_OBJECT
public:
 Receiver(){qRegisterMetaType<pocsag::Message>();}
 RadioConfig config;
 std::atomic_bool stopping{false};
 static QStringList devices();
 void stop(){stopping=true;wait();}
 ~Receiver(){stop();}
signals:
 void samples(QByteArray bytes);
 void decoded(quint32 ric,int function,int baud,QString type,QString text,int corrected,quint32 frequency,QString receiver);
 void frame(pocsag::Message message,quint32 frequency,QString receiver,QDateTime timestamp);
 void problem(QString message);
 void state(QString message);
protected:
 void run() override;
};
