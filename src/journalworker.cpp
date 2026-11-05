#include "journalworker.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QLockFile>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QCryptographicHash>
#include <QSet>
#include "journalpath.h"
void JournalWorker::request(QStringList disabled){QMutexLocker l(&mutex_);disabled_=disabled;++requested_;ready_.wakeOne();}
bool JournalWorker::flush(QStringList disabled,int timeout){QMutexLocker l(&mutex_);disabled_=disabled;auto target=++requested_;ready_.wakeOne();QElapsedTimer time;time.start();while(done_<target&&!closing_){int left=timeout-int(time.elapsed());if(left<=0)return false;finished_.wait(&mutex_,left);}return done_>=target;}
void JournalWorker::shutdown(){{QMutexLocker l(&mutex_);closing_=true;ready_.wakeAll();finished_.wakeAll();}wait();}
void JournalWorker::run(){QString name="journal-"+QUuid::createUuid().toString();{
 auto db=QSqlDatabase::addDatabase("QSQLITE",name);db.setDatabaseName(database_);if(!db.open()){emit problem("SQLite TXT : "+db.lastError().text());return;}{QSqlQuery p(db);p.exec("PRAGMA busy_timeout=50");p.finish();}
 QString receipts=QFileInfo(database_).absolutePath()+"/journal-receipts";QDir().mkpath(receipts);
 qint64 scanAfter=0;QSet<QString> blocked;
 for(;;){quint64 generation;QStringList disabled;{QMutexLocker l(&mutex_);while(!closing_&&done_==requested_)if(!ready_.wait(&mutex_,1000))++requested_;if(closing_)break;generation=requested_;disabled=disabled_;}
  for(auto type:disabled){QSqlQuery q(db);q.prepare("UPDATE journal_outbox SET done=2 WHERE done=0 AND message_id IN (SELECT id FROM messages WHERE type=?)");q.addBindValue(type);if(!q.exec())emit problem("SQLite TXT annulation différée : "+q.lastError().text());q.finish();}
  struct Entry{qint64 id;QString path;QByteArray line;};QVector<Entry> entries;{QSqlQuery q(db);q.setForwardOnly(true);q.prepare("SELECT id,path,line FROM journal_outbox WHERE done=0 AND id>? ORDER BY id LIMIT 1000");q.addBindValue(scanAfter);if(!q.exec())emit problem("SQLite TXT lecture différée : "+q.lastError().text());else{while(q.next())entries.push_back({q.value(0).toLongLong(),q.value(1).toString(),q.value(2).toByteArray()});if(q.lastError().isValid())emit problem(q.lastError().text());}q.finish();}
  if(scanAfter==0)blocked.clear();if(!entries.isEmpty())scanAfter=entries.last().id;int written=0;
  for(auto& e:entries){bool closing;{QMutexLocker l(&mutex_);closing=closing_;}if(closing)break;auto destinationKey=txtPathKey(e.path);if(blocked.contains(destinationKey))continue;try{
   if(!QDir().mkpath(QFileInfo(e.path).absolutePath()))throw std::runtime_error("Création dossier impossible");QLockFile lock(e.path+".pocsag.lock");if(!lock.tryLock(0))throw std::runtime_error("Fichier occupé");QFile file(e.path);if(!file.open(QIODevice::ReadWrite))throw std::runtime_error(file.errorString().toStdString());
   QString receiptPath=receipts+"/"+QString::fromLatin1(QCryptographicHash::hash(destinationKey.toUtf8(),QCryptographicHash::Sha256).toHex())+".json";QJsonObject receipt;{auto readPath=receiptPath;if(!QFileInfo::exists(readPath))readPath=receipts+"/"+QString::fromLatin1(QCryptographicHash::hash(e.path.toUtf8(),QCryptographicHash::Sha256).toHex())+".json";QFile r(readPath);if(r.open(QIODevice::ReadOnly))receipt=QJsonDocument::fromJson(r.readAll()).object();}qint64 lastId=receipt["id"].toString().toLongLong();bool delivered=e.id<lastId;
   if(e.id==lastId){qint64 offset=receipt["offset"].toString().toLongLong();file.seek(offset);auto tail=file.read(e.line.size());if(tail==e.line)delivered=true;else if(file.size()>=offset&&file.size()<offset+e.line.size()&&e.line.startsWith(tail)){if(!file.resize(offset))throw std::runtime_error("Reprise d'écriture partielle impossible");}else throw std::runtime_error("Conflit avec une modification externe du TXT ; aucune donnée écrasée");}
   // Legacy deliveries carried UUIDs. Keep their existing deduplication on upgrade only.
   if(!delivered&&e.id!=lastId&&e.line.startsWith('[')){file.seek(qMax(qint64(0),file.size()-qMax(qint64(65536),qint64(e.line.size())*2)));delivered=file.readAll().contains(e.line);}
   if(!delivered){qint64 offset=file.size();if(offset){file.seek(offset-1);if(file.read(1)!="\n"){file.seek(offset);if(file.write("\r\n")!=2||!file.flush())throw std::runtime_error("Séparation des lignes impossible");offset=file.size();}}
    if(e.id==lastId)offset=receipt["offset"].toString().toLongLong();else{receipt={{"id",QString::number(e.id)},{"offset",QString::number(offset)},{"sha256",QString::fromLatin1(QCryptographicHash::hash(e.line,QCryptographicHash::Sha256).toHex())}};QSaveFile r(receiptPath);auto bytes=QJsonDocument(receipt).toJson(QJsonDocument::Compact);if(!r.open(QIODevice::WriteOnly)||r.write(bytes)!=bytes.size()||!r.commit())throw std::runtime_error("Accusé TXT local impossible");}
    file.seek(offset);if(file.write(e.line)!=e.line.size()||!file.flush())throw std::runtime_error("Écriture TXT incomplète ; reprise en attente");}
   file.close();QSqlQuery ack(db);ack.prepare("UPDATE journal_outbox SET done=1 WHERE id=?");ack.addBindValue(e.id);if(!ack.exec())throw std::runtime_error(ack.lastError().text().toStdString());ack.finish();++written;
  }catch(const std::exception& error){blocked.insert(destinationKey);emit problem("TXT différé : "+e.path+" — "+QString::fromUtf8(error.what()));}}
  {QSqlQuery count("SELECT count(*) FROM journal_outbox WHERE done=0",db);if(count.next())emit settled(count.value(0).toInt());else emit problem(count.lastError().text());count.finish();}
  if(entries.size()<1000)scanAfter=0;
  {QMutexLocker l(&mutex_);done_=generation;if(entries.size()==1000)++requested_;finished_.wakeAll();}
 }
 db.close();}QSqlDatabase::removeDatabase(name);}
