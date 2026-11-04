#pragma once
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QStringList>
class JournalWorker:public QThread {
 Q_OBJECT
public:
 explicit JournalWorker(QString database,QObject* parent=nullptr):QThread(parent),database_(database){}
 ~JournalWorker(){shutdown();}
 void request(QStringList disabled={});
 bool flush(QStringList disabled={},int timeoutMs=10000);
 void shutdown();
signals:void problem(QString message);void settled(int pending);
protected:void run()override;
private:
 QString database_;QStringList disabled_;QMutex mutex_;QWaitCondition ready_,finished_;
 quint64 requested_=0,done_=0;bool closing_=false;
};
