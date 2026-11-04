#include "window.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QFile>
#include <QSettings>
#include <QDir>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QDialog>
#include <QTimer>
#include <iostream>
struct IntegrationChecks {
 static void require(bool b){if(!b)throw std::runtime_error("integration assertion failed");}
 static void run(){
  QTemporaryDir dir;require(dir.isValid());QString dbPath;{
   Window w(dir.path());dbPath=w.db_.databaseName();
   require(!w.allowTone_->isChecked());w.received({760833,0,1200,"TONE","",0});require(w.all_->rowCount()==0&&w.saved_->rowCount()==0);
   w.daily_->setChecked(false);
   w.sql("INSERT INTO capcodes(ric,alias,path) VALUES(760833,'Test','"+dir.path()+"/specific.txt')");
   w.logging_->setChecked(true);w.generalCopy_->setChecked(true);w.logPath_->setText(dir.path()+"/general.txt");
   w.received({760833,3,1200,"ALPHA","Message UTF-8 é",1});w.received({122121,0,512,"NUMERIC","12345",0});
   require(w.all_->rowCount()==2);require(w.saved_->rowCount()==1);require(w.saved_->index(0,4).data().toString()=="0760833");
   auto read=[](QString f){QFile file(f);if(!file.open(QIODevice::ReadOnly))return QByteArray{};return file.readAll();};
   auto general=read(dir.path()+"/general.txt");require(general.count("\r\n")==2);require(general.contains(QString("é").toUtf8()));require(read(dir.path()+"/specific.txt").count("\r\n")==1);
   w.sql("INSERT INTO profiles VALUES('same','"+dir.path()+"/specific.txt','760833',1)");
   w.received({760833,3,2400,"ALPHA","Deuxième",0});require(read(dir.path()+"/specific.txt").count("\r\n")==2);
   w.logPath_->setText(dir.path()+"/changed.txt");w.received({122121,0,512,"NUMERIC","99",0});require(read(dir.path()+"/changed.txt").count("\r\n")==1);require(read(dir.path()+"/general.txt")==general+read(dir.path()+"/specific.txt").split('\n').value(1)+'\n');
   w.sql("UPDATE capcodes SET active=0");w.refresh();require(w.saved_->rowCount()==0&&w.all_->rowCount()==4);
   w.sql("INSERT INTO capcodes(ric,alias) VALUES(122121,'New')");w.refresh();require(w.saved_->rowCount()==2);
   w.search_->setText("Deuxième");require(w.all_->rowCount()==1);w.search_->clear();
   w.sql("UPDATE journal_outbox SET done=0");auto before=read(dir.path()+"/specific.txt");w.flushJournal();require(read(dir.path()+"/specific.txt")==before);
   QFile blocked(dir.path()+"/blocked");require(blocked.open(QIODevice::WriteOnly));blocked.write("test");blocked.close();w.logPath_->setText(dir.path()+"/blocked/retry.txt");w.received({122121,0,512,"NUMERIC","88",0});QSqlQuery retry("SELECT count(*) FROM journal_outbox WHERE done=0",w.db_);require(retry.next()&&retry.value(0).toInt()==1);retry.finish();QFile::remove(dir.path()+"/blocked");w.flushJournal();require(read(dir.path()+"/blocked/retry.txt").count("\r\n")==1);
   QSqlQuery q("PRAGMA integrity_check",w.db_);require(q.next()&&q.value(0).toString()=="ok");
   q.finish();for(int i=0;i<300;++i)w.sql(QString("INSERT INTO messages(timestamp,frequency,baud,ric,function,type,text,corrected,receiver) VALUES('2026-11-04T22:00:00',169626000,1200,0,3,'ALPHA','backlog%1',0,'test')").arg(i));w.refresh();require(w.all_->index(0,7).data().toString()=="backlog299");while(w.all_->canFetchMore())w.all_->fetchMore();require(w.all_->rowCount()==305);
   for(qint64 hz:{1000,2500,5000,6250,8330,10000,12500,25000,50000,100000}){w.step_->setCurrentIndex(w.step_->findData(hz));w.frequency_->setValue(169.626);w.frequency_->stepUp();require(qRound64(w.frequency_->value()*1e6)==169626000+hz);w.frequency_->stepDown();require(qRound64(w.frequency_->value()*1e6)==169626000);QKeyEvent key(QEvent::KeyPress,Qt::Key_Up,Qt::NoModifier);QApplication::sendEvent(w.frequency_,&key);require(qRound64(w.frequency_->value()*1e6)==169626000+hz);}
   w.frequency_->setFocus();QWheelEvent wheel(QPointF(5,5),QPointF(5,5),QPoint(),QPoint(0,120),Qt::NoButton,Qt::NoModifier,Qt::NoScrollPhase,false);auto prior=qRound64(w.frequency_->value()*1e6);QApplication::sendEvent(w.frequency_,&wheel);require(qRound64(w.frequency_->value()*1e6)==prior+w.step_->currentData().toLongLong());
   w.allowTone_->setChecked(true);w.saveOptions();
   QString qa=qEnvironmentVariable("POCSAG_QA_DIR");if(!qa.isEmpty()){QDir().mkpath(qa);w.show();QApplication::processEvents();w.grab().save(qa+"/main.png");for(QString kind:{"capcodes","journal"}){QTimer::singleShot(200,&w,[&,kind]{auto dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());if(dialog){dialog->grab().save(qa+"/"+kind+".png");dialog->reject();}});if(kind=="capcodes")w.capcodes();else w.journal();}}
  }
  { Window restored(dir.path());while(restored.all_->canFetchMore())restored.all_->fetchMore();require(restored.all_->rowCount()==305&&restored.saved_->rowCount()==3);require(restored.allowTone_->isChecked()); }
  QTemporaryDir dailyDir;{
   Window w(dailyDir.path());w.logging_->setChecked(true);w.generalCopy_->setChecked(true);w.daily_->setChecked(true);w.root_->setText(dailyDir.path()+"/root");w.profileName_->setText("Belgique");
   for(int ric:{1,2})w.sql(QString("INSERT INTO capcodes(ric,alias,directory,filename,daily) VALUES(%1,'Test','%2','cap.txt',1)").arg(ric).arg(dailyDir.path()+"/cap"+QString::number(ric)));
   QFile blocked(dailyDir.path()+"/root");require(blocked.open(QIODevice::WriteOnly));blocked.write("blocked");blocked.close();
   auto before=QDateTime::fromString("2026-11-04T22:59:59Z",Qt::ISODate),after=QDateTime::fromString("2026-11-04T23:00:01Z",Qt::ISODate);
   w.received({1,3,1200,"ALPHA","BEFORE",0},169626000,"test",before);w.received({2,3,1200,"ALPHA","AFTER",0},169626000,"test",after);
   QSqlQuery queued("SELECT count(*) FROM journal_outbox WHERE done=0",w.db_);require(queued.next()&&queued.value(0).toInt()==2);queued.finish();QFile::remove(dailyDir.path()+"/root");w.flushJournal();
   for(QString path:{dailyDir.path()+"/root/Belgique/2026-11-04/Belgique-2026-11-04.txt",dailyDir.path()+"/root/Belgique/2026-11-05/Belgique-2026-11-05.txt",dailyDir.path()+"/cap1/2026-11-04/cap-2026-11-04.txt",dailyDir.path()+"/cap2/2026-11-05/cap-2026-11-05.txt"})require(QFileInfo::exists(path));
   require(Window::dailyPath("X","B",QDateTime::fromString("2026-07-01T22:00:01Z",Qt::ISODate)).contains("2026-07-02"));
   auto image=w.spectrum_->grab().toImage();require(image.pixelColor(image.width()/2,image.height()-5).red()>200);
  }
  QTemporaryDir filterDir;{
   Window w(filterDir.path());w.logging_->setChecked(true);w.daily_->setChecked(false);w.logPath_->setText(filterDir.path()+"/log.txt");
   w.received({1,0,1200,"TONE","",0});require(w.all_->rowCount()==0&&!QFileInfo::exists(w.logPath_->text()));
   w.allowTone_->setChecked(true);w.received({1,0,1200,"TONE","",0});require(w.all_->rowCount()==1);w.allowTone_->setChecked(false);require(w.all_->rowCount()==0);
   w.allowAlpha_->setChecked(false);w.received({1,3,1200,"ALPHA","FILTERED",0});QSqlQuery count("SELECT count(*) FROM messages",w.db_);require(count.next()&&count.value(0).toInt()==1);count.finish();
   w.sql("INSERT INTO journal_outbox(message_id,path,line) SELECT id,'"+filterDir.path()+"/old-tone.txt',X'544F4E450D0A' FROM messages WHERE type='TONE'");w.flushJournal();require(!QFileInfo::exists(filterDir.path()+"/old-tone.txt"));
  }
  QTemporaryDir migrationDir;{
   {auto legacy=QSqlDatabase::addDatabase("QSQLITE","legacy-fixture");legacy.setDatabaseName(migrationDir.path()+"/pocsag.db");require(legacy.open());QSqlQuery q(legacy);require(q.exec("CREATE TABLE capcodes(ric INTEGER PRIMARY KEY,alias TEXT,color TEXT,active INTEGER,show_bottom INTEGER,path TEXT,description TEXT)"));require(q.exec("INSERT INTO capcodes VALUES(42,'legacy','#00ffff',1,1,'C:/legacy.txt','keep')"));require(q.exec("CREATE TABLE profiles(name TEXT PRIMARY KEY,path TEXT,capcodes TEXT,enabled INTEGER)"));require(q.exec("INSERT INTO profiles VALUES('old','C:/old.txt','42',0)"));legacy.close();}QSqlDatabase::removeDatabase("legacy-fixture");
   Window migrated(migrationDir.path());QSqlQuery q("SELECT alias,path,directory,interpretation FROM capcodes WHERE ric=42",migrated.db_);require(q.next()&&q.value(0).toString()=="legacy"&&q.value(1).toString()=="C:/legacy.txt"&&q.value(2).toString().isEmpty()&&q.value(3).toString()=="INHERIT");q.finish();q.exec("SELECT count(*) FROM profiles");require(q.next()&&q.value(0).toInt()==1);
  }
 }
};
int main(int argc,char** argv){QApplication app(argc,argv);app.setOrganizationName("POCSAG-SDR-Pro-Tests");app.setApplicationName("Integration");QSettings::setPath(QSettings::NativeFormat,QSettings::UserScope,QDir::tempPath());try{IntegrationChecks::run();std::cout<<"PASS: SQLite persistence, capcode panels/filter, UTF-8 append, multiple logs, path deduplication, live log change, restore, integrity\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
