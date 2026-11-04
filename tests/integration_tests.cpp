#include "window.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QFile>
#include <QSettings>
#include <QDir>
#include <iostream>
struct IntegrationChecks {
 static void require(bool b){if(!b)throw std::runtime_error("integration assertion failed");}
 static void run(){
  QTemporaryDir dir;require(dir.isValid());QString dbPath;{
   Window w(dir.path());dbPath=w.db_.databaseName();
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
   w.logPath_->setText(dir.path()+"/missing/retry.txt");w.received({122121,0,512,"NUMERIC","88",0});QSqlQuery retry("SELECT count(*) FROM journal_outbox WHERE done=0",w.db_);require(retry.next()&&retry.value(0).toInt()==1);retry.finish();QDir().mkpath(dir.path()+"/missing");w.flushJournal();require(read(dir.path()+"/missing/retry.txt").count("\r\n")==1);
   QSqlQuery q("PRAGMA integrity_check",w.db_);require(q.next()&&q.value(0).toString()=="ok");
  }
  { Window restored(dir.path());require(restored.all_->rowCount()==5&&restored.saved_->rowCount()==3); }
 }
};
int main(int argc,char** argv){QApplication app(argc,argv);app.setOrganizationName("POCSAG-SDR-Pro-Tests");app.setApplicationName("Integration");QSettings::setPath(QSettings::NativeFormat,QSettings::UserScope,QDir::tempPath());try{IntegrationChecks::run();std::cout<<"PASS: SQLite persistence, capcode panels/filter, UTF-8 append, multiple logs, path deduplication, live log change, restore, integrity\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
