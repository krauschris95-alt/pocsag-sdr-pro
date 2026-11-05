#include "window.h"
#include <QtWidgets>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTimeZone>
#include <QDesktopServices>
#include <cmath>

QString Window::dailyPath(QString root,QString name,QDateTime timestamp){
 const QString date=timestamp.toTimeZone(QTimeZone("Europe/Brussels")).date().toString(Qt::ISODate);
 return QDir(root).filePath(name+"-"+date+".txt");
}
QString Window::generalJournalPath(QDateTime timestamp)const{
 return dailyPath(root_->text(),"Belgique",timestamp);
}
void Window::migrate(){
 auto column=[this](QString name,QString declaration){QSqlQuery q("PRAGMA table_info(capcodes)",db_);bool found=false;while(q.next())if(q.value(1).toString()==name)found=true;q.finish();if(!found)sql("ALTER TABLE capcodes ADD COLUMN "+name+" "+declaration);};
 if(!db_.transaction())throw std::runtime_error("Migration transaction failed");
 column("directory","TEXT DEFAULT ''");column("filename","TEXT DEFAULT ''");column("daily","INTEGER DEFAULT 0");column("interpretation","TEXT DEFAULT 'INHERIT'");column("retention_days","INTEGER DEFAULT 0");
 column("voice_enabled","INTEGER DEFAULT 0");column("pager_enabled","INTEGER DEFAULT 0");column("txt_enabled","INTEGER DEFAULT 1");column("txt_path","TEXT DEFAULT ''");
 sql("CREATE TABLE IF NOT EXISTS txt_profiles(id INTEGER PRIMARY KEY AUTOINCREMENT,name TEXT NOT NULL,path TEXT NOT NULL,enabled INTEGER NOT NULL DEFAULT 0)");sql("CREATE TABLE IF NOT EXISTS txt_profile_capcodes(profile_id INTEGER NOT NULL,ric INTEGER NOT NULL,PRIMARY KEY(profile_id,ric))");
 {QSqlQuery columns("PRAGMA table_info(messages)",db_);bool exists=false;while(columns.next())if(columns.value(1).toString()=="event_uid")exists=true;columns.finish();if(!exists)sql("ALTER TABLE messages ADD COLUMN event_uid TEXT");}sql("CREATE UNIQUE INDEX IF NOT EXISTS message_event_uid ON messages(event_uid) WHERE event_uid IS NOT NULL");
 {QSqlQuery fields("PRAGMA table_info(messages)",db_);QStringList names;while(fields.next())names<<fields.value(1).toString();fields.finish();for(QString name:{"raw_bits","alpha_candidate","numeric_candidate","decode_status","interpretation_policy","termination","raw_codewords"})if(!names.contains(name))sql("ALTER TABLE messages ADD COLUMN "+name+" TEXT DEFAULT ''");if(!names.contains("payload_complete"))sql("ALTER TABLE messages ADD COLUMN payload_complete INTEGER DEFAULT 1");}
 sql("CREATE INDEX IF NOT EXISTS capcode_numeric_ric ON capcodes(CAST(ric AS INTEGER))");sql("CREATE TABLE IF NOT EXISTS pager_jobs(event_uid TEXT PRIMARY KEY,envelope TEXT NOT NULL,audio_path TEXT NOT NULL DEFAULT '',generation_state TEXT NOT NULL DEFAULT 'pending',transfer_state TEXT NOT NULL DEFAULT 'disabled',attempts INTEGER NOT NULL DEFAULT 0,next_attempt_ms INTEGER NOT NULL DEFAULT 0,expires_ms INTEGER NOT NULL DEFAULT 0,last_error TEXT NOT NULL DEFAULT '')");sql("CREATE INDEX IF NOT EXISTS pager_due ON pager_jobs(next_attempt_ms)");sql("PRAGMA user_version=9");if(!db_.commit())throw std::runtime_error("Migration commit failed");
}
void Window::saveOptions(){
 if(!allowTone_)return;
 QSettings s(settingsFile_,QSettings::IniFormat);
 s.setValue("journals/generalEnabled",generalCopy_->isChecked());if(selectedLogging_){s.setValue("journals/selectedEnabled",selectedLogging_->isChecked());s.setValue("journals/selectedPath",selectedLogPath_->text());}s.setValue("types/ALPHA",allowAlpha_->isChecked());s.setValue("types/NUMERIC",allowNumeric_->isChecked());s.setValue("types/TONE",allowTone_->isChecked());s.setValue("types/DIAGNOSTIC",allowDiagnostic_->isChecked());
 s.setValue("interpretation",format_->currentText());s.setValue("stepHz",step_->currentData());s.setValue("daily",daily_->isChecked());s.setValue("root",root_->text());s.setValue("profile",profileName_->text());s.sync();
}
void Window::retune(){
 if(!selectedRadio_||selectedRadio_==&radio_)spectrum_->tuneFrequency(quint64(std::llround(frequency_->value()*1e6)));
 if(radio_.isRunning()&&radio_.config.file.isEmpty()){
  // Restart is explicit: flush/discard the old chain before assigning the new config.
  status_->setText("Réaccord : redémarrage de la chaîne SDR…");
  radio_.stop();QCoreApplication::sendPostedEvents(this,QEvent::MetaCall);start();
 }
}
void Window::setupOptions(QAction* startAction,QAction* stopAction,QAction* openAction,QAction* capAction,QAction* logAction,QMenu* viewMenu,QMenu* helpMenu){
 QSettings s(settingsFile_,QSettings::IniFormat);
 selectedLogging_=new QCheckBox(this);selectedLogging_->hide();selectedLogging_->setChecked(s.value("journals/selectedEnabled",false).toBool());selectedLogPath_=new QLineEdit(s.value("journals/selectedPath",QFileInfo(db_.databaseName()).absolutePath()+"/Interventions.txt").toString(),this);selectedLogPath_->hide();generalCopy_->setChecked(s.value("journals/generalEnabled",logging_->isChecked()&&generalCopy_->isChecked()).toBool());

 menuBar()->clear();auto file=menuBar()->addMenu("Fichier");file->addAction(openAction);file->addAction("Exporter tous les résultats…",this,[this]{exportTable(allView_);});file->addAction("Quitter",this,&QWidget::close);
 auto reception=menuBar()->addMenu("Réception");reception->addAction(startAction);reception->addAction(stopAction);reception->addAction("Actualiser les appareils",this,&Window::reloadDevices);
 auto rates=reception->addMenu("Débit POCSAG");auto group=new QActionGroup(this);group->setExclusive(true);for(QString rate:{"AUTO","512","1200","2400"}){auto a=rates->addAction(rate);a->setCheckable(true);a->setChecked(baud_->currentText()==rate);group->addAction(a);connect(a,&QAction::triggered,this,[this,rate]{baud_->setCurrentText(rate);});connect(baud_,&QComboBox::currentTextChanged,a,[a,rate](QString value){a->setChecked(value==rate);});}
 auto options=menuBar()->addMenu("Options");options->addAction("Fréquence SDR…",this,[this]{bool ok;double f=QInputDialog::getDouble(this,"Fréquence","MHz",frequency_->value(),.001,4294.967295,6,&ok);if(ok)frequency_->setValue(f);});options->addAction("Gain SDR…",this,[this]{bool ok;double g=QInputDialog::getDouble(this,"Gain","dB (gain supporté le plus proche)",gain_->value(),0,50,1,&ok);if(ok)gain_->setValue(g);});options->addAction("Correction PPM…",this,[this]{bool ok;int p=QInputDialog::getInt(this,"Correction","PPM",ppm_->value(),-200,200,1,&ok);if(ok)ppm_->setValue(p);});
 auto filters=menuBar()->addMenu("Filtres");auto types=filters->addMenu("Types de messages");
 auto typeAction=[&](QString title,QString key,bool initial){auto a=types->addAction(title);a->setCheckable(true);a->setChecked(s.value("types/"+key,initial).toBool());connect(a,&QAction::toggled,this,[this]{saveOptions();refresh();});return a;};
 allowAlpha_=typeAction("Alphanumériques","ALPHA",true);allowNumeric_=typeAction("Numériques","NUMERIC",true);allowTone_=typeAction("TONE ONLY","TONE",false);allowDiagnostic_=typeAction("Données non textuelles / diagnostics","DIAGNOSTIC",true);
 menuBar()->addMenu(viewMenu);menuBar()->addMenu("Capcodes")->addAction(capAction);menuBar()->addMenu("Journal")->addAction(logAction);menuBar()->addMenu(helpMenu);
 auto controls=new QToolBar("Décodage et accord",this);controls->hide();controls->setMovable(false);controls->addWidget(new QLabel("Interprétation :"));format_=new QComboBox;format_->addItems({"ALPHA","NUMERIC","AUTO","TONE"});format_->setCurrentText(s.value("interpretation","AUTO").toString());controls->addWidget(format_);
 format_->setToolTip("Politique réseau explicite ALPHA / NUMERIC / TONE. AUTO conserve les cas ambigus sans annonce vocale. Priorité au réglage individuel du capcode.");
 auto interp=options->addMenu("Interprétation des données");auto interpGroup=new QActionGroup(this);for(QString mode:{"ALPHA","NUMERIC","AUTO","TONE"}){auto a=interp->addAction(mode);a->setCheckable(true);a->setChecked(mode==format_->currentText());interpGroup->addAction(a);connect(a,&QAction::triggered,this,[this,mode]{format_->setCurrentText(mode);});connect(format_,&QComboBox::currentTextChanged,a,[a,mode](QString m){a->setChecked(m==mode);});}
 controls->addWidget(new QLabel("Pas :"));step_=new QComboBox;for(qint64 hz:{1000,2500,5000,6250,8330,10000,12500,25000,50000,100000})step_->addItem(QString::number(hz/1000.0,'g',6)+" kHz",hz);
 qint64 step=s.value("stepHz",1000).toLongLong();int stepIndex=step_->findData(step);if(stepIndex<0){step_->addItem(QString::number(step)+" Hz",step);stepIndex=step_->count()-1;}step_->setCurrentIndex(stepIndex);frequency_->setSingleStep(step/1e6);frequency_->setStepType(QAbstractSpinBox::DefaultStepType);controls->addWidget(step_);
 auto steps=reception->addMenu("Pas de fréquence");auto stepGroup=new QActionGroup(this);for(int i=0;i<step_->count();++i){auto a=steps->addAction(step_->itemText(i));a->setCheckable(true);a->setChecked(i==step_->currentIndex());stepGroup->addAction(a);connect(a,&QAction::triggered,this,[this,i]{step_->setCurrentIndex(i);});connect(step_,&QComboBox::currentIndexChanged,a,[a,i](int current){a->setChecked(current==i);});}
 auto custom=controls->addAction("Pas personnalisé…");connect(custom,&QAction::triggered,this,[this]{bool ok;int hz=QInputDialog::getInt(this,"Pas personnalisé","Hz",step_->currentData().toInt(),1,10000000,1,&ok);if(ok){int i=step_->findData(hz);if(i<0){step_->addItem(QString::number(hz)+" Hz",hz);i=step_->count()-1;}step_->setCurrentIndex(i);}});
 steps->addAction(custom);
 controls->addWidget(new QLabel("Zoom spectre :"));auto zoom=new QComboBox;zoom->addItems({"1×","2×","4×","8×"});controls->addWidget(zoom);connect(zoom,&QComboBox::currentIndexChanged,this,[this](int i){spectrum_->zoom=1<<i;spectrum_->update();});
 auto zoomMenu=viewMenu->addMenu("Zoom spectre");auto zoomGroup=new QActionGroup(this);for(int i=0;i<zoom->count();++i){auto a=zoomMenu->addAction(zoom->itemText(i));a->setCheckable(true);a->setChecked(i==0);zoomGroup->addAction(a);connect(a,&QAction::triggered,this,[zoom,i]{zoom->setCurrentIndex(i);});}
 daily_=new QCheckBox("Toute la Belgique — classement quotidien");daily_->setChecked(s.value("daily",true).toBool());root_=new QLineEdit(s.value("root",QFileInfo(db_.databaseName()).absolutePath()+"/logs").toString());profileName_=new QLineEdit(s.value("profile","Belgique").toString());root_->setToolTip("Dossier principal des enregistrements TXT");profileName_->setToolTip("Nom du sous-dossier et préfixe des fichiers journaliers");
 auto dailyBar=new QToolBar("Journaux quotidiens",this);dailyBar->hide();dailyBar->setMovable(false);dailyBar->addWidget(daily_);dailyBar->addWidget(root_);dailyBar->addWidget(profileName_);auto browse=dailyBar->addAction("Parcourir…");connect(browse,&QAction::triggered,this,[this]{auto dir=QFileDialog::getExistingDirectory(this,"Dossier principal TXT",root_->text());if(!dir.isEmpty())root_->setText(dir);});auto folder=dailyBar->addAction("Ouvrir le dossier actif");connect(folder,&QAction::triggered,this,[this]{QString dir=QFileInfo(generalJournalPath(QDateTime::currentDateTimeUtc())).absolutePath();if(QDir().mkpath(dir))QDesktopServices::openUrl(QUrl::fromLocalFile(dir));else failure("Impossible de créer le dossier : "+dir);});
 activePath_=new QLabel(this);activePath_->hide();auto updatePath=[this]{activePath_->setText("Toute la Belgique : "+QDir::toNativeSeparators(generalJournalPath(QDateTime::currentDateTimeUtc())));};auto timer=new QTimer(this);connect(timer,&QTimer::timeout,this,updatePath);timer->start(1000);updatePath();
 connect(daily_,&QCheckBox::toggled,this,[this,updatePath]{saveOptions();updatePath();});for(auto edit:{root_,profileName_})connect(edit,&QLineEdit::editingFinished,this,[this,updatePath]{if(profileName_->text().isEmpty()||profileName_->text().contains(QRegularExpression("[\\\\/:*?\"<>|]"))||profileName_->text()=="."||profileName_->text()==".."){profileName_->setText("Belgique");}saveOptions();updatePath();});
 connect(step_,&QComboBox::currentIndexChanged,this,[this]{frequency_->setSingleStep(step_->currentData().toLongLong()/1e6);saveOptions();});connect(format_,&QComboBox::currentTextChanged,this,[this]{saveOptions();});
 connect(frequency_,&QDoubleSpinBox::valueChanged,this,[this]{retune();});connect(baud_,&QComboBox::currentTextChanged,this,[this]{if(radio_.isRunning()&&radio_.config.file.isEmpty())retune();});connect(gain_,&QDoubleSpinBox::valueChanged,this,[this]{if(radio_.isRunning())retune();});connect(ppm_,&QDoubleSpinBox::valueChanged,this,[this]{if(radio_.isRunning())retune();});
 spectrum_->frequency=quint64(std::llround(frequency_->value()*1e6));
}
