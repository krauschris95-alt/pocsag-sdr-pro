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
void JournalWorker::request(QStringList disabled){QMutexLocker l(&mutex_);disabled_=disabled;++requested_;ready_.wakeOne();}
bool JournalWorker::flush(QStringList disabled,int timeout){QMutexLocker l(&mutex_);disabled_=disabled;auto target=++requested_;ready_.wakeOne();QElapsedTimer time;time.start();while(done_<target&&!closing_){int left=timeout-int(time.elapsed());if(left<=0)return false;finished_.wait(&mutex_,left);}return done_>=target;}
void JournalWorker::shutdown(){{QMutexLocker l(&mutex_);closing_=true;ready_.wakeAll();finished_.wakeAll();}wait();}
void JournalWorker::run(){
 QString name="journal-"+QUuid::createUuid().toString();{
 auto db=QSqlDatabase::addDatabase("QSQLITE",name);db.setDatabaseName(database_);if(!db.open()){emit problem("SQLite TXT : "+db.lastError().text());return;}QSqlQuery prag(db);prag.exec("PRAGMA busy_timeout=2000");
 for(;;){quint64 generation;QStringList disabled;{QMutexLocker l(&mutex_);while(!closing_&&done_==requested_)ready_.wait(&mutex_);if(closing_)break;generation=requested_;disabled=disabled_;}
  for(auto type:disabled){QSqlQuery q(db);q.prepare("UPDATE journal_outbox SET done=2 WHERE done=0 AND message_id IN (SELECT id FROM messages WHERE type=?)");q.addBindValue(type);if(!q.exec())emit problem(q.lastError().text());}
  struct Entry{qint64 id;QString path;QByteArray line;};QVector<Entry> entries;QSqlQuery q("SELECT id,path,line FROM journal_outbox WHERE done=0 ORDER BY id LIMIT 1000",db);while(q.next())entries.push_back({q.value(0).toLongLong(),q.value(1).toString(),q.value(2).toByteArray()});q.finish();
  for(auto& e:entries){if(isInterruptionRequested())break;if(!QDir().mkpath(QFileInfo(e.path).absolutePath())){emit problem("TXT : création dossier impossible — "+e.path);continue;}QLockFile lock(e.path+".pocsag.lock");if(!lock.tryLock(0)){emit problem("TXT verrouillé : "+e.path);continue;}QFile file(e.path);if(!file.open(QIODevice::ReadWrite|QIODevice::Append)){emit problem("TXT : "+e.path+" — "+file.errorString());continue;}file.seek(qMax(qint64(0),file.size()-qMax(qint64(65536),qint64(e.line.size())*2)));auto tail=file.readAll();if(!tail.contains(e.line)){if(!tail.isEmpty()&&!tail.endsWith('\n')&&file.write("\r\n")!=2)continue;if(file.write(e.line)!=e.line.size()||!file.flush()){emit problem("TXT : échec écriture — "+e.path);continue;}}QSqlQuery update(db);update.prepare("UPDATE journal_outbox SET done=1 WHERE id=?");update.addBindValue(e.id);if(!update.exec())emit problem(update.lastError().text());}
  QSqlQuery count("SELECT count(*) FROM journal_outbox WHERE done=0",db);count.next();emit settled(count.value(0).toInt());count.finish();{QMutexLocker l(&mutex_);done_=generation;finished_.wakeAll();}
 }
 db.close();}QSqlDatabase::removeDatabase(name);
}
