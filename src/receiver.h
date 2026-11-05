#pragma once
#include <QThread>
#include <QStringList>
#include <atomic>
#include <QDateTime>
#include <QMetaType>
#include <QMutex>
#include "decoder.h"
Q_DECLARE_METATYPE(pocsag::Message)
struct RadioConfig { quint32 frequency=169626000, rate=240000; int gain=420,ppm=0,index=0; QString file; int baud=0; QString capture;QString deviceKey;bool automaticGain=false;quint32 bandwidth=0;bool reconnect=true;int vgaGain=20;quint32 channelBandwidth=21000;double squelch=-160;QString antenna;QString label; };
struct RadioDevice {QString name,key;};
class Receiver:public QThread {
 Q_OBJECT
public:
 Receiver(){qRegisterMetaType<pocsag::Message>();}
 RadioConfig config;
 std::atomic_bool stopping{false};
 static QStringList devices();
 static QVector<RadioDevice> deviceInfos();
 void start(Priority priority=InheritPriority){stopping=false;lastLiveIq_=0;QThread::start(priority);}
 bool hasLiveIq()const;
 void stop();
 ~Receiver(){stop();}
signals:
 void samples(QByteArray bytes);
 void decoded(quint32 ric,int function,int baud,QString type,QString text,int corrected,quint32 frequency,QString receiver);
 void frame(pocsag::Message message,quint32 frequency,QString receiver,QDateTime timestamp);
 void problem(QString message);
 void state(QString message);
 void tuned(quint32 frequency,quint32 rate,quint32 bandwidth,QString identity);
protected:
 void run() override;
private:
 std::atomic<long long> lastLiveIq_{0};
 QMutex deviceMutex_;void* activeDevice_=nullptr;int(*cancelAsync_)(void*)=nullptr;
};
