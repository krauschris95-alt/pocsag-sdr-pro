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
 return QDir(root).filePath(name+"/"+date+"/"+name+"-"+date+".txt");
}
QString Window::generalJournalPath(QDateTime timestamp)const{
 return daily_&&daily_->isChecked()?dailyPath(root_->text(),profileName_->text(),timestamp):logPath_->text();
}
void Window::migrate(){
 auto column=[this](QString name,QString declaration){QSqlQuery q("PRAGMA table_info(capcodes)",db_);bool found=false;while(q.next())if(q.value(1).toString()==name)found=true;q.finish();if(!found)sql("ALTER TABLE capcodes ADD COLUMN "+name+" "+declaration);};
 if(!db_.transaction())throw std::runtime_error("Migration transaction failed");
 column("directory","TEXT DEFAULT ''");column("filename","TEXT DEFAULT ''");column("daily","INTEGER DEFAULT 0");column("interpretation","TEXT DEFAULT 'INHERIT'");column("retention_days","INTEGER DEFAULT 0");
 column("voice_enabled","INTEGER DEFAULT 0");
 sql("PRAGMA user_version=3");if(!db_.commit())throw std::runtime_error("Migration commit failed");
}
void Window::saveOptions(){
 if(!allowTone_)return;
 QSettings s(settingsFile_,QSettings::IniFormat);
 s.setValue("types/ALPHA",allowAlpha_->isChecked());s.setValue("types/NUMERIC",allowNumeric_->isChecked());s.setValue("types/TONE",allowTone_->isChecked());s.setValue("types/DIAGNOSTIC",allowDiagnostic_->isChecked());
 s.setValue("interpretation",format_->currentText());s.setValue("stepHz",step_->currentData());s.setValue("daily",daily_->isChecked());s.setValue("root",root_->text());s.setValue("profile",profileName_->text());s.sync();
}
void Window::retune(){
 spectrum_->tuneFrequency(quint64(std::llround(frequency_->value()*1e6)));
 if(radio_.isRunning()&&radio_.config.file.isEmpty()){
  // Restart is explicit: flush/discard the old chain before assigning the new config.
  status_->setText("Réaccord : redémarrage de la chaîne SDR…");
  radio_.stop();QCoreApplication::sendPostedEvents(this,QEvent::MetaCall);start();
 }
}
void Window::setupOptions(QAction* startAction,QAction* stopAction,QAction* openAction,QAction* capAction,QAction* logAction,QMenu* viewMenu,QMenu* helpMenu){
 QSettings s(settingsFile_,QSettings::IniFormat);
 menuBar()->clear();auto file=menuBar()->addMenu("Fichier");file->addAction(openAction);file->addAction("Exporter tous les résultats…",this,[this]{exportTable(allView_);});file->addAction("Quitter",this,&QWidget::close);
 auto reception=menuBar()->addMenu("Réception");reception->addAction(startAction);reception->addAction(stopAction);reception->addAction("Actualiser les appareils",this,[this]{if(!radio_.isRunning()){devices_->clear();devices_->addItems(Receiver::devices());}});
 auto rates=reception->addMenu("Débit POCSAG");auto group=new QActionGroup(this);group->setExclusive(true);for(QString rate:{"AUTO","512","1200","2400"}){auto a=rates->addAction(rate);a->setCheckable(true);a->setChecked(baud_->currentText()==rate);group->addAction(a);connect(a,&QAction::triggered,this,[this,rate]{baud_->setCurrentText(rate);});connect(baud_,&QComboBox::currentTextChanged,a,[a,rate](QString value){a->setChecked(value==rate);});}
 auto options=menuBar()->addMenu("Options");options->addAction("Fréquence SDR…",this,[this]{bool ok;double f=QInputDialog::getDouble(this,"Fréquence","MHz",frequency_->value(),24,1766,6,&ok);if(ok)frequency_->setValue(f);});options->addAction("Gain SDR…",this,[this]{bool ok;double g=QInputDialog::getDouble(this,"Gain","dB (gain supporté le plus proche)",gain_->value(),0,50,1,&ok);if(ok)gain_->setValue(g);});options->addAction("Correction PPM…",this,[this]{bool ok;int p=QInputDialog::getInt(this,"Correction","PPM",ppm_->value(),-200,200,1,&ok);if(ok)ppm_->setValue(p);});
 auto filters=menuBar()->addMenu("Filtres");auto types=filters->addMenu("Types de messages");
 auto typeAction=[&](QString title,QString key,bool initial){auto a=types->addAction(title);a->setCheckable(true);a->setChecked(s.value("types/"+key,initial).toBool());connect(a,&QAction::toggled,this,[this]{saveOptions();refresh();});return a;};
 allowAlpha_=typeAction("Alphanumériques","ALPHA",true);allowNumeric_=typeAction("Numériques","NUMERIC",true);allowTone_=typeAction("TONE ONLY","TONE",false);allowDiagnostic_=typeAction("Invalides / diagnostics","DIAGNOSTIC",false);
 menuBar()->addMenu(viewMenu);menuBar()->addMenu("Capcodes")->addAction(capAction);menuBar()->addMenu("Journal")->addAction(logAction);menuBar()->addMenu(helpMenu);
 auto controls=new QToolBar("Décodage et accord",this);addToolBarBreak();addToolBar(controls);controls->setMovable(false);controls->addWidget(new QLabel("Interprétation :"));format_=new QComboBox;format_->addItems({"ALPHA","NUMERIC","AUTO"});format_->setCurrentText(s.value("interpretation","ALPHA").toString());controls->addWidget(format_);
 format_->setToolTip("ALPHA par défaut, sans déduction par fonction. AUTO conserve les ambiguïtés comme diagnostics. Surcharge possible par capcode.");
 auto interp=options->addMenu("Interprétation des données");auto interpGroup=new QActionGroup(this);for(QString mode:{"ALPHA","NUMERIC","AUTO"}){auto a=interp->addAction(mode);a->setCheckable(true);a->setChecked(mode==format_->currentText());interpGroup->addAction(a);connect(a,&QAction::triggered,this,[this,mode]{format_->setCurrentText(mode);});connect(format_,&QComboBox::currentTextChanged,a,[a,mode](QString m){a->setChecked(m==mode);});}
 controls->addWidget(new QLabel("Pas :"));step_=new QComboBox;for(qint64 hz:{1000,2500,5000,6250,8330,10000,12500,25000,50000,100000})step_->addItem(QString::number(hz/1000.0,'g',6)+" kHz",hz);
 qint64 step=s.value("stepHz",1000).toLongLong();int stepIndex=step_->findData(step);if(stepIndex<0){step_->addItem(QString::number(step)+" Hz",step);stepIndex=step_->count()-1;}step_->setCurrentIndex(stepIndex);frequency_->setSingleStep(step/1e6);frequency_->setStepType(QAbstractSpinBox::DefaultStepType);controls->addWidget(step_);
 auto custom=controls->addAction("Pas personnalisé…");connect(custom,&QAction::triggered,this,[this]{bool ok;int hz=QInputDialog::getInt(this,"Pas personnalisé","Hz",step_->currentData().toInt(),1,10000000,1,&ok);if(ok){int i=step_->findData(hz);if(i<0){step_->addItem(QString::number(hz)+" Hz",hz);i=step_->count()-1;}step_->setCurrentIndex(i);}});
 controls->addWidget(new QLabel("Zoom spectre :"));auto zoom=new QComboBox;zoom->addItems({"1×","2×","4×","8×"});controls->addWidget(zoom);connect(zoom,&QComboBox::currentIndexChanged,this,[this](int i){spectrum_->zoom=1<<i;spectrum_->update();});
 daily_=new QCheckBox("Toute la Belgique — classement quotidien");daily_->setChecked(s.value("daily",true).toBool());root_=new QLineEdit(s.value("root",QFileInfo(db_.databaseName()).absolutePath()+"/logs").toString());profileName_=new QLineEdit(s.value("profile","Belgique").toString());root_->setToolTip("Dossier principal des enregistrements TXT");profileName_->setToolTip("Nom du sous-dossier et préfixe des fichiers journaliers");
 auto dailyBar=new QToolBar("Journaux quotidiens",this);addToolBarBreak();addToolBar(dailyBar);dailyBar->setMovable(false);dailyBar->addWidget(daily_);dailyBar->addWidget(root_);dailyBar->addWidget(profileName_);auto browse=dailyBar->addAction("Parcourir…");connect(browse,&QAction::triggered,this,[this]{auto dir=QFileDialog::getExistingDirectory(this,"Dossier principal TXT",root_->text());if(!dir.isEmpty())root_->setText(dir);});auto folder=dailyBar->addAction("Ouvrir le dossier actif");connect(folder,&QAction::triggered,this,[this]{QString dir=QFileInfo(generalJournalPath(QDateTime::currentDateTimeUtc())).absolutePath();if(QDir().mkpath(dir))QDesktopServices::openUrl(QUrl::fromLocalFile(dir));else failure("Impossible de créer le dossier : "+dir);});
 activePath_=new QLabel;statusBar()->addWidget(activePath_,1);auto updatePath=[this]{activePath_->setText("Toute la Belgique : "+QDir::toNativeSeparators(generalJournalPath(QDateTime::currentDateTimeUtc())));};auto timer=new QTimer(this);connect(timer,&QTimer::timeout,this,updatePath);timer->start(1000);updatePath();
 connect(daily_,&QCheckBox::toggled,this,[this,updatePath]{saveOptions();updatePath();});for(auto edit:{root_,profileName_})connect(edit,&QLineEdit::editingFinished,this,[this,updatePath]{if(profileName_->text().isEmpty()||profileName_->text().contains(QRegularExpression("[\\\\/:*?\"<>|]"))||profileName_->text()=="."||profileName_->text()==".."){profileName_->setText("Belgique");}saveOptions();updatePath();});
 connect(step_,&QComboBox::currentIndexChanged,this,[this]{frequency_->setSingleStep(step_->currentData().toLongLong()/1e6);saveOptions();});connect(format_,&QComboBox::currentTextChanged,this,[this]{saveOptions();});
 connect(frequency_,&QDoubleSpinBox::valueChanged,this,[this]{retune();});connect(baud_,&QComboBox::currentTextChanged,this,[this]{if(radio_.isRunning()&&radio_.config.file.isEmpty())retune();});connect(gain_,&QDoubleSpinBox::valueChanged,this,[this]{if(radio_.isRunning())retune();});connect(ppm_,&QDoubleSpinBox::valueChanged,this,[this]{if(radio_.isRunning())retune();});
 spectrum_->frequency=quint64(std::llround(frequency_->value()*1e6));
}
