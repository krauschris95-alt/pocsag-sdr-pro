#pragma once
#include "speech.h"
#include <QJsonObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
struct PagerAck{bool accepted=false;QString eventId,error;};
// No network implementation before the PrivateRadio API is validated.
class PagerTransport{public:virtual ~PagerTransport()=default;virtual bool encryptedAndAuthenticated()const=0;virtual PagerAck deliver(const QJsonObject&,const QByteArray&)=0;};
struct PagerSettings{QString audioRoot;int expirySeconds=3600;bool relay=false;SpeechSettings speech;};
class PagerWorker:public QThread{
 Q_OBJECT
public:explicit PagerWorker(QString database,std::unique_ptr<SpeechBackend> backend={},std::shared_ptr<PagerTransport> transport={}):database_(std::move(database)),backend_(std::move(backend)),transport_(std::move(transport)){}~PagerWorker(){shutdown();}
 void configure(PagerSettings settings);void request();void shutdown();
signals:void diagnostic(QString);void settled();
protected:void run()override;
private:QString database_;std::unique_ptr<SpeechBackend> backend_;std::shared_ptr<PagerTransport> transport_;PagerSettings settings_;QMutex mutex_;QWaitCondition wake_;bool closing_=false;std::atomic_bool cancel_=false;
};
struct WavInfo{bool valid=false;quint32 sampleRate=0;quint16 channels=0,bits=0;quint64 samples=0;double seconds=0;QString sha256;};
WavInfo inspectPcmWav(QString path);
