#include "window.h"
#include <QtWidgets>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <iostream>
#include <source_location>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
struct IntegrationChecks {
 static void require(bool ok,std::source_location at=std::source_location::current()){if(!ok)throw std::runtime_error("capcode check line "+std::to_string(at.line()));}
 static void edit(Window& w,std::function<void(QDialog*,QTableWidget*,QPushButton*)> action){std::exception_ptr error;QTimer dismiss;QObject::connect(&dismiss,&QTimer::timeout,[]{if(auto box=qobject_cast<QMessageBox*>(QApplication::activeModalWidget()))box->accept();});dismiss.start(10);QTimer::singleShot(10,&w,[&]{auto dlg=qobject_cast<QDialog*>(QApplication::activeModalWidget());try{require(dlg);action(dlg,dlg->findChild<QTableWidget*>(),dlg->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save));}catch(...){error=std::current_exception();dlg->reject();}});w.capcodes();if(error)std::rethrow_exception(error);}
 static void add(QDialog* d){for(auto b:d->findChildren<QPushButton*>())if(b->text()=="Ajouter"){b->click();return;}require(false);}
 static QString value(Window& w,QString column){QSqlQuery q("SELECT "+column+" FROM capcodes WHERE ric=146035",w.db_);require(q.next());return q.value(0).toString();}
 static void reproduce(){QTemporaryDir root;QString dir=root.path();{
  Window w(dir);require(!w.radio_.hasLiveIq());w.updateSdrIndicator();require(w.sdrIndicator_->text().contains("INACTIF"));
  edit(w,[&](auto d,auto t,auto save){add(d);require(t->item(0,14));t->item(0,0)->setText("0146035");t->item(0,1)->setText("Ambulance Z3");t->item(0,14)->setCheckState(Qt::Checked);t->item(0,7)->setText(dir+"/nouveau/dossier");t->item(0,8)->setText("interventions.txt");save->click();require(!d->isVisible());});
  require(value(w,"alias")=="Ambulance Z3"&&value(w,"voice_enabled")=="1");require(!QFileInfo::exists(dir+"/nouveau/dossier/interventions.txt"));
  edit(w,[&](auto d,auto t,auto save){t->item(0,16)->setCheckState(Qt::Checked);save->click();require(!d->isVisible());});require(value(w,"pager_enabled")=="1");
  edit(w,[&](auto d,auto t,auto save){t->item(0,1)->setText("Alias modifié");t->item(0,4)->setCheckState(Qt::Unchecked);t->item(0,14)->setCheckState(Qt::Unchecked);save->click();require(!d->isVisible());});require(value(w,"alias")=="Alias modifié"&&value(w,"voice_enabled")=="0");
  QFile existing(dir+"/existant.txt");require(existing.open(QIODevice::WriteOnly));existing.write("CONSERVER\r\n");existing.close();
  for(QString path:{dir+"/existant.txt",dir+"/nouveau-fichier.txt"})edit(w,[&](auto d,auto t,auto save){t->item(0,7)->setText("");t->item(0,5)->setText(path);save->click();require(!d->isVisible());});
  require(existing.open(QIODevice::ReadOnly)&&existing.readAll()=="CONSERVER\r\n");existing.close();require(!QFileInfo::exists(dir+"/nouveau-fichier.txt"));

  QFile blocked(dir+"/bloque");require(blocked.open(QIODevice::WriteOnly));blocked.close();
  for(QString invalid:{"", "abc", "2097152", "-1"})edit(w,[&](auto d,auto t,auto save){t->item(0,0)->setText(invalid);save->click();require(d->isVisible()&&t->item(0,0)->text()==invalid);d->reject();});

  edit(w,[&](auto d,auto t,auto save){add(d);t->item(1,0)->setText("0146035");save->click();require(d->isVisible()&&t->rowCount()==2);d->reject();});
  edit(w,[&](auto d,auto t,auto save){delete t->takeItem(0,14);save->click();require(d->isVisible());d->reject();});
  w.sql("CREATE TRIGGER save_failure BEFORE INSERT ON capcodes BEGIN SELECT RAISE(ABORT,'Injected SQLite failure'); END");edit(w,[&](auto d,auto t,auto save){t->item(0,1)->setText("Echec SQL");save->click();require(d->isVisible());d->reject();});require(value(w,"alias")=="Alias modifié");w.sql("DROP TRIGGER save_failure");
  edit(w,[&](auto d,auto t,auto save){for(int i=1;i<4;++i){add(d);t->item(i,0)->setText(QString::number(146035+i));}t->item(0,4)->setCheckState(Qt::Checked);t->item(0,14)->setCheckState(Qt::Checked);save->click();require(!d->isVisible());});
  QFile backup(dir+"/capcodes-backup.json");require(backup.open(QIODevice::ReadOnly));auto json=QJsonDocument::fromJson(backup.readAll());require(json.isArray()&&json.array().size()==1);backup.close();
  w.logging_->setChecked(true);w.generalCopy_->setChecked(true);w.daily_->setChecked(false);w.root_->setText(dir+"/general");w.selectedLogging_->setChecked(true);w.selectedLogPath_->setText(dir+"/nouveau-fichier.txt");w.received({146035,0,1200,"ALPHA","Message de test sauvegarde",0},169626000,"test");w.flushJournal();require(w.all_->rowCount()==1&&w.saved_->rowCount()==1);QFile txt(dir+"/nouveau-fichier.txt");require(txt.open(QIODevice::ReadOnly)&&txt.readAll().count("Message de test sauvegarde")==1);txt.close();
  if(QCoreApplication::arguments().contains("--hardware")){w.frequency_->setValue(169.626);w.start();QEventLoop loop;bool green=false,saveOk=false;QTimer monitor;QObject::connect(&monitor,&QTimer::timeout,[&]{w.updateSdrIndicator();if(w.radio_.hasLiveIq())green=w.sdrIndicator_->text().contains("SDR ACTIF");});monitor.start(100);QTimer::singleShot(3000,&w,[&]{try{edit(w,[&](auto d,auto t,auto save){t->item(0,1)->setText("Essai Save avec SDR réel");save->click();require(!d->isVisible());});saveOk=w.radio_.isRunning();}catch(...){saveOk=false;}});QTimer::singleShot(60000,&loop,&QEventLoop::quit);loop.exec();w.radio_.stop();w.updateSdrIndicator();w.flushJournal();QSqlQuery q("SELECT count(*) FROM messages WHERE receiver NOT IN ('test','')",w.db_);q.next();int received=q.value(0).toInt();QFile general(w.generalJournalPath(QDateTime::currentDateTimeUtc()));require(general.open(QIODevice::ReadOnly));bool written=general.readAll().count("\r\n")>=received;std::cout<<"Hardware IQ green="<<green<<" save_running="<<saveOk<<" real_texts="<<received<<" TXT="<<written<<" red_after_stop="<<!w.radio_.hasLiveIq()<<'\n';require(green&&saveOk&&received>0&&written&&w.sdrIndicator_->text().contains("INACTIF"));}
 }{Window restored(dir);QSqlQuery q("SELECT count(*) FROM capcodes",restored.db_);require(q.next()&&q.value(0).toInt()==4);require(value(restored,"voice_enabled")=="1"&&value(restored,"show_bottom")=="1"&&value(restored,"path")==dir+"/nouveau-fichier.txt"&&value(restored,"pager_enabled")=="1");}
 }
};
int main(int argc,char**argv){QApplication app(argc,argv);app.setOrganizationName("POCSAG-TEST-015");app.setApplicationName("POCSAG-TEST-015");try{IntegrationChecks::reproduce();std::cout<<"PASS capcode Save, validation, rollback, paths, backup and persistence\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
