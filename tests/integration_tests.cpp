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
#include <QSemaphore>
#include <QtWidgets>
#include <source_location>
#include "pagercontract.h"
struct VoiceState {QMutex mutex;QStringList said;QSemaphore began;std::atomic_bool release=false;};
struct VoiceBackend:SpeechBackend {VoiceState& state;explicit VoiceBackend(VoiceState& s):state(s){}void configure(const SpeechSettings&)override{}void speak(const QString& text,const std::atomic_bool& cancel)override{{QMutexLocker lock(&state.mutex);state.said<<text;}state.began.release();while(!state.release&&!cancel)QThread::msleep(2);}};
struct IntegrationChecks {
 static void require(bool b,std::source_location at=std::source_location::current()){if(!b)throw std::runtime_error("integration assertion failed at line "+std::to_string(at.line()));}
 static void compactLayout(){
  QTemporaryDir dir;double savedRatio=0;{
   Window w(dir.path());w.resize(1440,900);w.show();QApplication::processEvents();
   require(w.tabs_->count()==2&&w.spectrum_->parentWidget()==w.tabs_->widget(1));
   auto sizes=w.messageSplit_->sizes();double ratio=double(sizes[0])/(sizes[0]+sizes[1]);require(ratio>.57&&ratio<.63);
   require(w.allView_->height()+w.savedView_->height()>w.centralWidget()->height()*.88);
   require(w.savedView_->height()>250&&w.allView_->height()>350);
   require(!w.root_->isVisible()&&!w.logPath_->isVisible()&&!w.step_->isVisible()&&!w.format_->isVisible());
   int bars=0;for(auto bar:w.findChildren<QToolBar*>())if(bar->isVisible())++bars;require(bars==1);
   auto handle=w.messageSplit_->handle(1);auto local=QPointF(handle->rect().center());auto global=QPointF(handle->mapToGlobal(local.toPoint()));
   QMouseEvent press(QEvent::MouseButtonPress,local,global,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(handle,&press);
   QMouseEvent move(QEvent::MouseMove,local-QPointF(0,100),global-QPointF(0,100),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(handle,&move);
   QMouseEvent release(QEvent::MouseButtonRelease,local,global-QPointF(0,100),Qt::LeftButton,Qt::NoButton,Qt::NoModifier);QApplication::sendEvent(handle,&release);QApplication::processEvents();
   auto moved=w.messageSplit_->sizes();require(moved[0]<sizes[0]-50&&moved[1]>sizes[1]+50);savedRatio=double(moved[0])/(moved[0]+moved[1]);w.storePreferences();
  }{
   Window w(dir.path());w.show();QApplication::processEvents();auto sizes=w.messageSplit_->sizes();require(std::abs(double(sizes[0])/(sizes[0]+sizes[1])-savedRatio)<.02);
   w.sql("INSERT INTO capcodes(ric,alias,active,show_bottom) VALUES(146035,'Essais interface',1,1),(146054,'Inactif',0,1)");
   for(int i=0;i<70;++i)w.received({i%3==0?146035u:i%3==1?146054u:2009976u,0,1200,"ALPHA","Message de validation interface — texte de test non issu du réseau radio.",0},169626000,"test");
   require(w.all_->rowCount()==70&&w.saved_->rowCount()==24);
   require(w.all_->columnCount()==9&&w.saved_->columnCount()==8);
   QApplication::processEvents();require(w.allView_->verticalScrollBar()->maximum()>0&&w.savedView_->verticalScrollBar()->maximum()>0);
   QString qa=qEnvironmentVariable("POCSAG_QA_DIR");if(!qa.isEmpty()){QDir().mkpath(qa);w.messageSplit_->setSizes({600,400});QApplication::processEvents();w.grab().save(qa+"/compact-messages.png");w.tabs_->setCurrentIndex(1);QApplication::processEvents();require(w.spectrum_->height()>w.centralWidget()->height()*.90);w.grab().save(qa+"/compact-spectrum.png");}
  }
 }
 static void run(){
  compactLayout();
  {QTemporaryDir data;Window w(data.path());QFile compressed(QString::fromUtf8(FIXTURE_DIRECTORY)+"/1200-alpha.iqz");require(compressed.open(QIODevice::ReadOnly));auto iq=qUncompress(compressed.readAll());require(!iq.isEmpty());for(QString name:{"a","b"}){QFile f(data.path()+"/"+name+".cu8");require(f.open(QIODevice::WriteOnly));f.write(iq);}w.logging_->setChecked(true);w.daily_->setChecked(false);w.logPath_->setText(data.path()+"/multi.txt");RadioConfig config;config.file=data.path()+"/a.cu8";config.frequency=169626000;auto a=w.addReceiver(config);config.file=data.path()+"/b.cu8";config.frequency=169650000;auto b=w.addReceiver(config);w.selectReceiver(b);w.tabs_->setCurrentIndex(1);require(a->isRunning()&&b->isRunning());w.tabs_->setCurrentIndex(0);QEventLoop loop;QTimer poll;QObject::connect(&poll,&QTimer::timeout,&loop,[&]{if(!a->isRunning()&&!b->isRunning())loop.quit();});poll.start(20);QTimer::singleShot(5000,&loop,&QEventLoop::quit);loop.exec();QApplication::processEvents();w.flushJournal();QSqlQuery sources("SELECT count(*),count(DISTINCT receiver),count(DISTINCT frequency) FROM messages WHERE text='REFERENCE 012 FUNCTION TEST'",w.db_);require(sources.next()&&sources.value(0).toInt()==2&&sources.value(1).toInt()==2&&sources.value(2).toInt()==2);require(w.frequencyDisplay_->text().startsWith("169.650000"));require(!a->hasLiveIq()&&!b->hasLiveIq());QFile log(data.path()+"/multi.txt");require(log.open(QIODevice::ReadOnly));auto text=log.readAll();require(text.count("Source=IQ: ")==2&&text.count("REFERENCE 012 FUNCTION TEST")==2);w.received({42,0,1200,"ALPHA","Percent %8 %9 %1 conservés",0},169626000,"Test source");w.flushJournal();log.seek(0);require(log.readAll().contains(QString("Percent %8 %9 %1 conservés").toUtf8()));}
  require(!pagerEligible(false,true,true)&&!pagerEligible(true,false,true)&&!pagerEligible(true,true,false)&&pagerEligible(true,true,true));auto envelope=pagerEnvelope("event-1",146035,"Ambulance Z3",QDateTime::currentDateTimeUtc(),169626000,"RTL test","Texte complet");require(envelope.value("utterance").toString()=="Ambulance Z3. Texte complet"&&envelope.value("message_id").toString()=="event-1");
  {QTemporaryDir folder;Window w(folder.path());w.logging_->setChecked(true);w.generalCopy_->setChecked(false);w.daily_->setChecked(false);w.logPath_->setText(folder.path()+"/general.txt");w.sql("INSERT INTO capcodes(ric,alias,path,txt_enabled) VALUES(42,'Test','"+folder.path()+"/specific.txt',0)");w.received({42,0,1200,"ALPHA","Sans TXT",0},169626000,"Test");w.flushJournal();require(w.all_->rowCount()==1&&!QFileInfo::exists(folder.path()+"/general.txt")&&!QFileInfo::exists(folder.path()+"/specific.txt"));w.sql("UPDATE capcodes SET txt_enabled=1");w.received({42,0,1200,"ALPHA","Avec TXT spécifique",0},169626000,"Test");w.flushJournal();require(QFileInfo::exists(folder.path()+"/specific.txt")&&!QFileInfo::exists(folder.path()+"/general.txt"));}
  QTemporaryDir dir;require(dir.isValid());QString dbPath;{
   Window w(dir.path());require(w.tabs_->count()==2);require(w.tabs_->tabText(0)=="Messages");require(w.tabs_->tabText(1)=="Spectre / Waterfall");w.messageSplit_->setSizes({300,200});dbPath=w.db_.databaseName();
   require(!w.allowTone_->isChecked());w.received({760833,0,1200,"TONE","",0});require(w.all_->rowCount()==0&&w.saved_->rowCount()==0);
   w.daily_->setChecked(false);
   w.sql("INSERT INTO capcodes(ric,alias,path) VALUES(760833,'Test','"+dir.path()+"/specific.txt')");
   w.logging_->setChecked(true);w.generalCopy_->setChecked(true);w.logPath_->setText(dir.path()+"/general.txt");
   w.received({760833,3,1200,"ALPHA","Message UTF-8 é",1});w.received({122121,0,512,"NUMERIC","12345",0});
   require(w.all_->rowCount()==2);require(w.saved_->rowCount()==1);require(w.saved_->index(0,0).data().toString()=="0760833");
   auto read=[&w](QString f){w.flushJournal();QFile file(f);if(!file.open(QIODevice::ReadOnly))return QByteArray{};return file.readAll();};
   auto general=read(dir.path()+"/general.txt");require(general.count("\r\n")==2);require(general.contains(QString("é").toUtf8()));require(read(dir.path()+"/specific.txt").count("\r\n")==1);
   w.sql("INSERT INTO profiles VALUES('same','"+dir.path()+"/specific.txt','760833',1)");
   w.received({760833,3,2400,"ALPHA","Deuxième",0});require(read(dir.path()+"/specific.txt").count("\r\n")==2);
   w.logPath_->setText(dir.path()+"/changed.txt");w.received({122121,0,512,"NUMERIC","99",0});require(read(dir.path()+"/changed.txt").count("\r\n")==1);require(read(dir.path()+"/general.txt")==general+read(dir.path()+"/specific.txt").split('\n').value(1)+'\n');
   w.sql("UPDATE capcodes SET active=0");w.refresh();require(w.saved_->rowCount()==0&&w.all_->rowCount()==4);
   w.sql("INSERT INTO capcodes(ric,alias) VALUES(122121,'New')");w.refresh();require(w.saved_->rowCount()==2);
   w.search_->setText("Deuxième");require(w.all_->rowCount()==1);w.search_->clear();
   w.sql("UPDATE journal_outbox SET done=0");auto before=read(dir.path()+"/specific.txt");w.flushJournal();require(read(dir.path()+"/specific.txt")==before);
   QFile blocked(dir.path()+"/blocked");require(blocked.open(QIODevice::WriteOnly));blocked.write("test");blocked.close();w.logPath_->setText(dir.path()+"/blocked/retry.txt");w.received({122121,0,512,"NUMERIC","88",0});w.flushJournal();QSqlQuery retry("SELECT count(*) FROM journal_outbox WHERE done=0",w.db_);require(retry.next()&&retry.value(0).toInt()==1);retry.finish();QFile::remove(dir.path()+"/blocked");w.flushJournal();require(read(dir.path()+"/blocked/retry.txt").count("\r\n")==1);
   QSqlQuery q("PRAGMA integrity_check",w.db_);require(q.next()&&q.value(0).toString()=="ok");
   q.finish();for(int i=0;i<300;++i)w.sql(QString("INSERT INTO messages(timestamp,frequency,baud,ric,function,type,text,corrected,receiver) VALUES('2026-11-04T22:00:00',169626000,1200,0,3,'ALPHA','backlog%1',0,'test')").arg(i));w.refresh();require(w.all_->index(0,8).data().toString()=="backlog299");while(w.all_->canFetchMore())w.all_->fetchMore();require(w.all_->rowCount()==305);
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
   w.flushJournal();QSqlQuery queued("SELECT count(*) FROM journal_outbox WHERE done=0",w.db_);require(queued.next()&&queued.value(0).toInt()==2);queued.finish();QFile::remove(dailyDir.path()+"/root");w.flushJournal();
   for(QString path:{dailyDir.path()+"/root/Belgique/2026-11-04/Belgique-2026-11-04.txt",dailyDir.path()+"/root/Belgique/2026-11-05/Belgique-2026-11-05.txt",dailyDir.path()+"/cap1/2026-11-04/cap-2026-11-04.txt",dailyDir.path()+"/cap2/2026-11-05/cap-2026-11-05.txt"})require(QFileInfo::exists(path));
   require(Window::dailyPath("X","B",QDateTime::fromString("2026-07-01T22:00:01Z",Qt::ISODate)).contains("2026-07-02"));
   auto image=w.spectrum_->grab().toImage();require(image.pixelColor(image.width()/2,image.height()-5).red()>200);
  }
  QTemporaryDir filterDir;{
   Window w(filterDir.path());w.logging_->setChecked(true);w.daily_->setChecked(false);w.logPath_->setText(filterDir.path()+"/log.txt");
   w.received({1,0,1200,"TONE","",0});require(w.all_->rowCount()==0&&!QFileInfo::exists(w.logPath_->text()));
   w.allowTone_->setChecked(true);w.received({1,0,1200,"TONE","",0});require(w.all_->rowCount()==1);w.allowTone_->setChecked(false);require(w.all_->rowCount()==0);
   w.allowAlpha_->setChecked(false);w.received({1,3,1200,"ALPHA","FILTERED",0});QSqlQuery count("SELECT count(*) FROM messages",w.db_);require(count.next()&&count.value(0).toInt()==2);count.finish();
   w.sql("INSERT INTO journal_outbox(message_id,path,line) SELECT id,'"+filterDir.path()+"/old-tone.txt',X'544F4E450D0A' FROM messages WHERE type='TONE'");w.flushJournal();require(!QFileInfo::exists(filterDir.path()+"/old-tone.txt"));
  }
  QTemporaryDir migrationDir;{
   {auto legacy=QSqlDatabase::addDatabase("QSQLITE","legacy-fixture");legacy.setDatabaseName(migrationDir.path()+"/pocsag.db");require(legacy.open());QSqlQuery q(legacy);require(q.exec("CREATE TABLE capcodes(ric INTEGER PRIMARY KEY,alias TEXT,color TEXT,active INTEGER,show_bottom INTEGER,path TEXT,description TEXT)"));require(q.exec("INSERT INTO capcodes VALUES(42,'legacy','#00ffff',1,1,'C:/legacy.txt','keep')"));require(q.exec("CREATE TABLE profiles(name TEXT PRIMARY KEY,path TEXT,capcodes TEXT,enabled INTEGER)"));require(q.exec("INSERT INTO profiles VALUES('old','C:/old.txt','42',0)"));legacy.close();}QSqlDatabase::removeDatabase("legacy-fixture");
   Window migrated(migrationDir.path());QSqlQuery q("SELECT alias,path,directory,interpretation FROM capcodes WHERE ric=42",migrated.db_);require(q.next()&&q.value(0).toString()=="legacy"&&q.value(1).toString()=="C:/legacy.txt"&&q.value(2).toString().isEmpty()&&q.value(3).toString()=="INHERIT");q.finish();q.exec("SELECT count(*) FROM profiles");require(q.next()&&q.value(0).toInt()==1);
  }
  QTemporaryDir voiceDir;{VoiceState state;Window w(voiceDir.path(),std::make_unique<VoiceBackend>(state));w.speechSettings_.enabled=true;w.speech_.configure(w.speechSettings_);w.sql("INSERT INTO capcodes(ric,alias,voice_enabled) VALUES(146035,'Ambulance Z3',1)");w.logging_->setChecked(true);w.daily_->setChecked(false);w.logPath_->setText(voiceDir.path()+"/voice.txt");w.received({146035,3,1200,"ALPHA","Texte complet du premier message.",0});require(state.began.tryAcquire(1,2000));require(w.all_->rowCount()==1&&w.saved_->rowCount()==1);w.received({146035,3,1200,"ALPHA","Deuxième message.",0});require(w.speech_.pending()==1);w.sql("UPDATE capcodes SET voice_enabled=0");w.received({146035,3,1200,"ALPHA","Sans voix individuelle.",0});require(w.speech_.pending()==1);w.speechSettings_.enabled=false;w.speech_.configure(w.speechSettings_);w.received({146035,3,1200,"ALPHA","Sans voix générale.",0});w.flushJournal();require(w.all_->rowCount()==4&&w.saved_->rowCount()==4);QFile log(voiceDir.path()+"/voice.txt");require(log.open(QIODevice::ReadOnly)&&log.readAll().count("\r\n")==4);{QMutexLocker lock(&state.mutex);require(state.said==QStringList{"Ambulance Z3. Texte complet du premier message."});}w.storePreferences();}
 }
};
int main(int argc,char** argv){QApplication app(argc,argv);app.setOrganizationName("POCSAG-SDR-Pro-Tests");app.setApplicationName("Integration");QSettings::setPath(QSettings::NativeFormat,QSettings::UserScope,QDir::tempPath());try{IntegrationChecks::run();std::cout<<"PASS: SQLite persistence, capcode panels/filter, UTF-8 append, multiple logs, path deduplication, live log change, restore, integrity\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
