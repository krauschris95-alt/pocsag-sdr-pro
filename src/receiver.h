#pragma once
#include <QThread>
#include <QStringList>
#include <atomic>
struct RadioConfig { quint32 frequency=146950000, rate=240000; int gain=420,ppm=0,index=0; QString file; int baud=0; };
class Receiver:public QThread {
 Q_OBJECT
public:
 RadioConfig config;
 std::atomic_bool stopping{false};
 static QStringList devices();
 void stop(){stopping=true;wait();}
 ~Receiver(){stop();}
signals:
 void samples(QByteArray bytes);
 void decoded(quint32 ric,int function,int baud,QString type,QString text,int corrected,quint32 frequency,QString receiver);
 void problem(QString message);
 void state(QString message);
protected:
 void run() override;
};
