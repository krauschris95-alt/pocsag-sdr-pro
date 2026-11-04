#pragma once
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSet>
#include <QVector>
#include <deque>
#include <atomic>
#include <memory>
struct SpeechChoice { QString id,name; bool french=false; };
struct SpeechSettings { bool enabled=false; QString voice,output; int volume=80,rate=0,limit=50; bool dropOldest=false; };
struct SpeechJob { QString eventId,alias,text; };
class SpeechBackend {
public:
 virtual ~SpeechBackend()=default;
 virtual void configure(const SpeechSettings&)=0;
 virtual void speak(const QString&,const std::atomic_bool& cancel)=0;
};
class SpeechService:public QThread {
 Q_OBJECT
public:
 explicit SpeechService(std::unique_ptr<SpeechBackend> backend={},QObject* parent=nullptr);
 ~SpeechService();
 static QVector<SpeechChoice> voices();
 static QVector<SpeechChoice> outputs();
 static QString utterance(const SpeechJob& job);
 void configure(SpeechSettings settings);
 bool enqueue(SpeechJob job);
 void stopSpeaking();
 void shutdown();
 int pending();
signals:
 void problem(QString message);
 void speaking(QString eventId);
 void completed(QString eventId);
 void skipped(QString reason);
protected:void run()override;
private:
 QMutex mutex_;QWaitCondition ready_;std::deque<SpeechJob> jobs_;QSet<QString> seen_;
 SpeechSettings settings_;std::unique_ptr<SpeechBackend> backend_;
 bool closing_=false;std::atomic_bool cancel_{false};
};
