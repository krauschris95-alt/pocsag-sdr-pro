#include "window.h"
#include <QtWidgets>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSettings>
#include <QStandardPaths>
#include <QSaveFile>
#include <QTimeZone>
#include <cmath>
#include <numbers>
#include <set>
class MessageModel:public QSqlQueryModel {
public:
 QHash<QString,QColor> colors;
 QVariant data(const QModelIndex& idx,int role=Qt::DisplayRole)const override{
  if(role==Qt::ForegroundRole&&(idx.column()==4||idx.column()==7)){
   auto color=colors.value(QSqlQueryModel::data(index(idx.row(),4)).toString());if(color.isValid())return color;
  }return QSqlQueryModel::data(idx,role);
 }
 void apply(QString query,const QSqlDatabase& db){base_=query;db_=db;reload();}
 void sort(int column,Qt::SortOrder order)override{column_=column;order_=order;if(!base_.isEmpty())reload();}
private:
 QString base_;QSqlDatabase db_;int column_=-1;Qt::SortOrder order_=Qt::AscendingOrder;
 void reload(){auto query=base_;if(column_>=0){query=query.left(query.lastIndexOf("ORDER BY"))+"ORDER BY "+QString::number(column_+1)+(order_==Qt::AscendingOrder?" ASC":" DESC");}setQuery(query,db_);}
};
void Spectrum::updateIq(const QByteArray& b){
 constexpr int N=1024;if(b.size()<N*2)return;std::vector<std::complex<float>> a(N);
 for(int i=0;i<N;++i)a[i]=std::complex<float>((uchar(b[2*i])-127.5f)/128,(uchar(b[2*i+1])-127.5f)/128)*float(.5-.5*cos(2*std::numbers::pi*i/(N-1)));
 for(int i=1,j=0;i<N;++i){int k=N>>1;for(;j&k;k>>=1)j^=k;j^=k;if(i<j)std::swap(a[i],a[j]);}
 for(int len=2;len<=N;len*=2){auto wlen=std::polar(1.f,float(-2*std::numbers::pi/len));for(int i=0;i<N;i+=len){std::complex<float>w=1;for(int j=0;j<len/2;++j){auto u=a[i+j],v=a[i+j+len/2]*w;a[i+j]=u+v;a[i+j+len/2]=u-v;w*=wlen;}}}
 levels_.resize(N);if(waterfall_.isNull()) {waterfall_=QImage(N,80,QImage::Format_RGB32);waterfall_.fill(Qt::black);}
 for(int y=79;y>0;--y)memcpy(waterfall_.scanLine(y),waterfall_.scanLine(y-1),N*4);
 for(int i=0;i<N;++i){float db=20*log10(std::max(1e-8f,std::abs(a[(i+N/2)%N])/512));levels_[i]=db;float p=std::clamp((db+100)/80.f,0.f,1.f);waterfall_.setPixelColor(i,0,QColor::fromHsvF(.67-p*.67,1,p));}update();
}
void Spectrum::paintEvent(QPaintEvent*){
 QPainter p(this);p.fillRect(rect(),QColor("#020e20"));int h=height()*2/3;p.setPen(QColor("#20334f"));for(int i=0;i<8;++i)p.drawLine(i*width()/8,0,i*width()/8,h);for(int i=0;i<5;++i){int y=i*h/5;p.drawLine(0,y,width(),y);p.drawText(4,y+14,QString::number(-20-i*20)+" dBFS");}
 auto marker=[&]{p.setPen(QPen(QColor("#ff3030"),2));p.drawLine(width()/2,0,width()/2,height());p.drawText(width()/2+5,18,QString::number(frequency/1e6,'f',6)+" MHz");};
 if(levels_.isEmpty()){p.setPen(Qt::lightGray);p.drawText(rect(),Qt::AlignCenter,"Aucune donnée IQ — spectre en attente");marker();return;}
 int bins=std::max(2,int(levels_.size()/zoom)),first=(levels_.size()-bins)/2;QPainterPath line;for(int j=0;j<bins;++j){int i=first+j;double x=double(j)*width()/(bins-1),y=std::clamp((-levels_[i]-20)/100.f,0.f,1.f)*h;if(!j)line.moveTo(x,y);else line.lineTo(x,y);}p.setPen(QColor("#42dcff"));p.drawPath(line);p.drawImage(QRect(0,h,width(),height()-h),waterfall_,QRect(first,0,bins,waterfall_.height()));marker();
}
void Window::sql(const QString& text){QSqlQuery q(db_);if(!q.exec(text))throw std::runtime_error(q.lastError().text().toStdString());}
static QTableView* table(QSqlQueryModel* model){auto t=new QTableView;t->setModel(model);t->setSelectionBehavior(QAbstractItemView::SelectRows);t->setSelectionMode(QAbstractItemView::ExtendedSelection);t->setAlternatingRowColors(true);t->verticalHeader()->hide();t->verticalHeader()->setDefaultSectionSize(25);t->horizontalHeader()->setStretchLastSection(true);t->horizontalHeader()->setSortIndicator(-1,Qt::AscendingOrder);t->setSortingEnabled(true);return t;}
Window::Window(QString storageDirectory){
 setWindowTitle("POCSAG SDR Pro — 0.1.2");resize(1440,980);
 auto dir=storageDirectory.isEmpty()?QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation):storageDirectory;QDir().mkpath(dir);
 settingsFile_=dir+"/preferences.ini";
 db_=QSqlDatabase::addDatabase("QSQLITE");db_.setDatabaseName(dir+"/pocsag.db");if(!db_.open())throw std::runtime_error(db_.lastError().text().toStdString());
 sql("PRAGMA journal_mode=WAL");sql("PRAGMA busy_timeout=5000");
 sql("CREATE TABLE IF NOT EXISTS messages(id INTEGER PRIMARY KEY, timestamp TEXT, frequency INTEGER, baud INTEGER, ric INTEGER, function INTEGER, type TEXT, text TEXT, corrected INTEGER, receiver TEXT)");
 sql("CREATE INDEX IF NOT EXISTS messages_ric ON messages(ric)");
 sql("CREATE TABLE IF NOT EXISTS capcodes(ric INTEGER PRIMARY KEY CHECK(ric BETWEEN 0 AND 2097151),alias TEXT,color TEXT DEFAULT '#47d7ff',active INTEGER DEFAULT 1,show_bottom INTEGER DEFAULT 1,path TEXT DEFAULT '',description TEXT DEFAULT '')");
 sql("CREATE TABLE IF NOT EXISTS profiles(name TEXT PRIMARY KEY,path TEXT,capcodes TEXT DEFAULT '',enabled INTEGER DEFAULT 0)");
 sql("CREATE TABLE IF NOT EXISTS journal_outbox(id INTEGER PRIMARY KEY,message_id INTEGER,path TEXT,line BLOB,done INTEGER DEFAULT 0)");
 sql("CREATE INDEX IF NOT EXISTS outbox_pending ON journal_outbox(done,id)");
 migrate();
 all_=new MessageModel;all_->setParent(this);saved_=new MessageModel;saved_->setParent(this);
 auto central=new QWidget;auto layout=new QVBoxLayout(central);setCentralWidget(central);
 auto toolbar=addToolBar("Réception");toolbar->setMovable(false);
 auto add=[&](QString title,auto fn){auto a=toolbar->addAction(title);connect(a,&QAction::triggered,this,fn);return a;};
 auto startAction=add("▶ Démarrer",[this]{start();});auto stopAction=add("■ Arrêter",[this]{radio_.stop();status_->setText("Réception arrêtée");});
 auto openAction=add("Ouvrir IQ…",[this]{auto f=QFileDialog::getOpenFileName(this,"Ouvrir IQ CU8 (I/Q non signé 8 bits)",{},"IQ (*.cu8 *.bin);;Tous (*)");if(!f.isEmpty())start(f);});
 add("Actualiser SDR",[this]{devices_->clear();devices_->addItems(Receiver::devices());});
 auto capAction=add("Capcodes",[this]{capcodes();});auto logAction=add("Journal",[this]{journal();});
 add("Statistiques",[this]{QSqlQuery q("SELECT count(*),sum(corrected),count(DISTINCT ric) FROM messages",db_);q.next();QMessageBox::information(this,"Statistiques enregistrées",QString("Messages : %1\nBits corrigés : %2\nCapcodes distincts : %3").arg(q.value(0).toString(),q.value(1).toString(),q.value(2).toString()));});
 auto fileMenu=menuBar()->addMenu("Fichier");fileMenu->addAction(openAction);fileMenu->addAction("Exporter tous les résultats…",this,[this]{exportTable(allView_);});fileMenu->addAction("Quitter",this,&QWidget::close);
 auto reception=menuBar()->addMenu("Réception");reception->addAction(startAction);reception->addAction(stopAction);
 menuBar()->addMenu("Capcodes")->addAction(capAction);menuBar()->addMenu("Décodage")->addAction("Redémarrer la chaîne",this,[this]{start(radio_.config.file);});menuBar()->addMenu("Outils")->addAction(logAction);
 auto viewMenu=menuBar()->addMenu("Affichage");auto help=menuBar()->addMenu("Aide");help->addAction("À propos",this,[this]{QMessageBox::information(this,"POCSAG SDR Pro","Version 0.1.2\nRTL-SDR direct / IQ CU8.\nInterprétation sélectionnable globalement et par capcode.\nALPHA par défaut ; TONE ONLY désactivé par défaut.\nAucun texte radio n'est transmis à un serveur.\nHackRF et multicanal non disponibles.");});
 auto settings=new QHBoxLayout;layout->addLayout(settings);devices_=new QComboBox;devices_->addItems(Receiver::devices());settings->addWidget(new QLabel("Appareil SDR :"));settings->addWidget(devices_);
 frequency_=new QDoubleSpinBox;frequency_->setRange(24,1766);frequency_->setDecimals(6);frequency_->setValue(169.626);frequency_->setSuffix(" MHz");frequency_->setKeyboardTracking(false);settings->addWidget(frequency_);
 gain_=new QDoubleSpinBox;gain_->setRange(0,50);gain_->setValue(42);gain_->setSuffix(" dB");settings->addWidget(gain_);ppm_=new QDoubleSpinBox;ppm_->setRange(-200,200);ppm_->setSuffix(" PPM");settings->addWidget(ppm_);
 baud_=new QComboBox;baud_->addItems({"AUTO","512","1200","2400"});settings->addWidget(baud_);settings->addWidget(new QLabel("IQ : 240 kS/s • canal centré"));status_=new QLabel("Réception arrêtée");settings->addWidget(status_,1);
 spectrum_=new Spectrum;layout->addWidget(spectrum_);
 auto split=new QSplitter(Qt::Vertical);layout->addWidget(split,1);
 auto panel=[&](QString title,QSqlQueryModel* model,QTableView*& t,QLineEdit*& search){auto w=new QWidget;auto l=new QVBoxLayout(w);l->addWidget(new QLabel(title));auto bar=new QHBoxLayout;search=new QLineEdit;search->setPlaceholderText("Rechercher message, capcode ou alias");bar->addWidget(search);auto copy=new QPushButton("Copier");auto exp=new QPushButton("Exporter CSV");bar->addWidget(copy);bar->addWidget(exp);l->addLayout(bar);t=table(model);l->addWidget(t);auto tv=t;connect(copy,&QPushButton::clicked,this,[this,tv]{copyTable(tv);});connect(exp,&QPushButton::clicked,this,[this,tv]{exportTable(tv);});connect(search,&QLineEdit::textChanged,this,[this]{refresh();});connect(tv,&QTableView::doubleClicked,this,[this,tv](QModelIndex idx){QMessageBox::information(this,"Message complet",tv->model()->index(idx.row(),tv->model()->columnCount()-1).data().toString());});split->addWidget(w);return w;};
 panel("TOUS LES MESSAGES POCSAG",all_,allView_,search_);auto bottom=panel("MESSAGES DES CAPCODES ENREGISTRÉS",saved_,savedView_,savedSearch_);split->setSizes({420,220});auto visible=viewMenu->addAction("Panneau des capcodes enregistrés");visible->setCheckable(true);visible->setChecked(true);connect(visible,&QAction::toggled,bottom,&QWidget::setVisible);
 auto scroll=viewMenu->addAction("Défilement automatique");scroll->setCheckable(true);scroll->setChecked(true);connect(scroll,&QAction::toggled,this,[this](bool v){autoscroll_=v;});
 auto logs=new QHBoxLayout;layout->addLayout(logs);logging_=new QCheckBox("Enregistrement TXT");generalCopy_=new QCheckBox("Copie journal général");logPath_=new QLineEdit;auto choose=new QPushButton("Choisir un autre fichier…");logs->addWidget(logging_);logs->addWidget(generalCopy_);logs->addWidget(logPath_,1);logs->addWidget(choose);connect(choose,&QPushButton::clicked,this,[this]{auto f=QFileDialog::getSaveFileName(this,"Journal texte (ajout sans effacement)",logPath_->text(),"Texte (*.txt)");if(!f.isEmpty())logPath_->setText(f);});count_=new QLabel;statusBar()->addPermanentWidget(count_);
 QSettings s;frequency_->setValue(s.value("frequency",169.626).toDouble());gain_->setValue(s.value("gain",42).toDouble());ppm_->setValue(s.value("ppm",0).toDouble());logPath_->setText(s.value("log",dir+"/general.txt").toString());logging_->setChecked(s.value("logging",false).toBool());generalCopy_->setChecked(s.value("generalCopy",true).toBool());restoreGeometry(s.value("geometry").toByteArray());
 setupOptions(startAction,stopAction,openAction,capAction,logAction,viewMenu,help);
 connect(&radio_,&Receiver::samples,spectrum_,&Spectrum::updateIq);connect(&radio_,&Receiver::frame,this,[this](pocsag::Message message,quint32 frequency,QString receiver,QDateTime timestamp){received(std::move(message),frequency,receiver,timestamp);});connect(&radio_,&Receiver::problem,this,&Window::failure);connect(&radio_,&Receiver::state,status_,&QLabel::setText);refresh();
 auto retryJournal=new QTimer(this);retryJournal->setInterval(10000);connect(retryJournal,&QTimer::timeout,this,&Window::flushJournal);retryJournal->start();QTimer::singleShot(0,this,&Window::flushJournal);
 setStyleSheet("QWidget{background:#071425;color:#e6eefb;font-family:'Segoe UI';font-size:12px} QToolBar,QMenuBar,QHeaderView::section{background:#122841;padding:6px} QPushButton,QToolButton{background:#193753;border:1px solid #53708b;border-radius:4px;padding:6px} QLineEdit,QComboBox,QDoubleSpinBox{background:#0b1e33;border:1px solid #4c647d;padding:5px} QTableView{alternate-background-color:#102238;selection-background-color:#005dc8;gridline-color:#193047} QLabel{padding:3px} QSplitter::handle{background:#285074;height:5px}");
}
Window::~Window(){radio_.stop();saveOptions();QSettings s;s.setValue("frequency",frequency_->value());s.setValue("gain",gain_->value());s.setValue("ppm",ppm_->value());s.setValue("log",logPath_->text());s.setValue("logging",logging_->isChecked());s.setValue("generalCopy",generalCopy_->isChecked());s.setValue("geometry",saveGeometry());}
void Window::failure(QString message){status_->setText(message);QMessageBox::warning(this,"Erreur",message);}
void Window::start(QString file){
 radio_.stop();selectedBaud_=baud_->currentText().toInt();
 radio_.config={quint32(std::llround(frequency_->value()*1e6)),240000,int(gain_->value()*10),int(ppm_->value()),devices_->currentIndex(),file,selectedBaud_};radio_.start();
}
void Window::received(pocsag::Message m,quint32 frequency,QString receiver,QDateTime timestamp){
 if(!m.rawBits.empty()){
  QString mode=format_->currentText();QSqlQuery config(db_);config.prepare("SELECT interpretation FROM capcodes WHERE ric=?");config.addBindValue(m.ric);config.exec();if(config.next()&&config.value(0).toString()!="INHERIT")mode=config.value(0).toString();m=pocsag::interpret(std::move(m),mode.toStdString());
 }
 if((m.type=="TONE"&&!allowTone_->isChecked())||(m.type=="ALPHA"&&!allowAlpha_->isChecked())||(m.type=="NUMERIC"&&!allowNumeric_->isChecked())||(m.type=="DIAGNOSTIC"&&!allowDiagnostic_->isChecked()))return;
 if(!timestamp.isValid())timestamp=QDateTime::currentDateTimeUtc();
 if(!frequency)frequency=radio_.config.frequency;if(receiver.isEmpty())receiver=radio_.config.file.isEmpty()?devices_->currentText():radio_.config.file;
 if(!db_.transaction()){failure(db_.lastError().text());return;}
 QSqlQuery q(db_);q.prepare("INSERT INTO messages(timestamp,frequency,baud,ric,function,type,text,corrected,receiver) VALUES(?,?,?,?,?,?,?,?,?)");auto now=timestamp.toTimeZone(QTimeZone("Europe/Brussels")).toString(Qt::ISODateWithMs);q.addBindValue(now);q.addBindValue(frequency);q.addBindValue(m.baud);q.addBindValue(m.ric);q.addBindValue(m.function);q.addBindValue(QString::fromStdString(m.type));q.addBindValue(QString::fromStdString(m.text));q.addBindValue(m.corrected);q.addBindValue(receiver);if(!q.exec()){db_.rollback();failure(q.lastError().text());return;}
 if(logging_->isChecked()){
  std::set<QString> paths;QSqlQuery cap(db_);cap.prepare("SELECT path,directory,filename,daily FROM capcodes WHERE ric=? AND active=1");cap.addBindValue(m.ric);cap.exec();if(cap.next()){
   QString path=cap.value(0).toString(),directory=cap.value(1).toString(),filename=cap.value(2).toString();if(!directory.isEmpty()){if(filename.isEmpty())filename=QString("%1.txt").arg(m.ric,7,10,QChar('0'));if(cap.value(3).toBool()){QString date=timestamp.toTimeZone(QTimeZone("Europe/Brussels")).date().toString(Qt::ISODate);filename=date+"/"+QFileInfo(filename).completeBaseName()+"-"+date+".txt";}path=QDir(directory).filePath(filename);}if(!path.isEmpty())paths.insert(QFileInfo(path).absoluteFilePath());
  }
  if(generalCopy_->isChecked()||paths.empty()){QString path=generalJournalPath(timestamp);if(!path.isEmpty())paths.insert(QFileInfo(path).absoluteFilePath());}
  QSqlQuery profiles("SELECT path,capcodes FROM profiles WHERE enabled=1",db_);while(profiles.next()){auto rics=profiles.value(1).toString().split(',',Qt::SkipEmptyParts);bool match=rics.isEmpty();for(const auto& r:rics)if(r.trimmed().toUInt()==m.ric)match=true;if(match&&!profiles.value(0).toString().isEmpty())paths.insert(QFileInfo(profiles.value(0).toString()).absoluteFilePath());}
  auto line=("["+QUuid::createUuid().toString(QUuid::WithoutBraces)+"] "+QString("%1 | %2 MHz | %3 | %4 | F%5 | %6 | %7\r\n").arg(now).arg(frequency/1e6,0,'f',6).arg(m.baud).arg(m.ric,7,10,QChar('0')).arg(m.function).arg(QString::fromStdString(m.type),QString::fromStdString(m.text).replace('\n',"\\n").replace('\r',"\\r"))).toUtf8();
  for(auto& path:paths){QSqlQuery queued(db_);queued.prepare("INSERT INTO journal_outbox(message_id,path,line) VALUES(?,?,?)");queued.addBindValue(q.lastInsertId());queued.addBindValue(path);queued.addBindValue(line);if(!queued.exec()){db_.rollback();failure(queued.lastError().text());return;}}
 }if(!db_.commit()){db_.rollback();failure(db_.lastError().text());return;}flushJournal();refresh();
}
void Window::flushJournal(){
 if(allowTone_){QStringList disabled;if(!allowAlpha_->isChecked())disabled<<"'ALPHA'";if(!allowNumeric_->isChecked())disabled<<"'NUMERIC'";if(!allowTone_->isChecked())disabled<<"'TONE'";if(!allowDiagnostic_->isChecked())disabled<<"'DIAGNOSTIC'";if(!disabled.isEmpty()){QSqlQuery cancel(db_);if(!cancel.exec("UPDATE journal_outbox SET done=2 WHERE done=0 AND message_id IN (SELECT id FROM messages WHERE type IN ("+disabled.join(',')+"))")){status_->setText("Échec du filtrage de la file TXT : "+cancel.lastError().text());return;}}}
 QSqlQuery pending("SELECT id,path,line FROM journal_outbox WHERE done=0 ORDER BY id LIMIT 1000",db_);
 struct Entry{QVariant id;QString path;QByteArray line;};QVector<Entry> entries;while(pending.next())entries.append({pending.value(0),pending.value(1).toString(),pending.value(2).toByteArray()});pending.finish();
 for(const auto& e:entries){
  if(!QDir().mkpath(QFileInfo(e.path).absolutePath())){status_->setText("Journal en attente : création du dossier impossible — "+e.path);continue;}
  QLockFile lock(e.path+".pocsag.lock");if(!lock.tryLock(0)){status_->setText("Journal en attente (verrouillé) : "+e.path);continue;}
  QFile f(e.path);if(!f.open(QIODevice::ReadWrite|QIODevice::Append)){status_->setText("Journal en attente : "+e.path+" — "+f.errorString());continue;}
  f.seek(std::max(qint64(0),f.size()-std::max(qint64(65536),qint64(e.line.size())*2)));auto tail=f.readAll();
  if(!tail.contains(e.line)){
   if(!tail.isEmpty()&&!tail.endsWith('\n')&&f.write("\r\n")!=2)continue;
   if(f.write(e.line)!=e.line.size()||!f.flush()){status_->setText("Échec écriture TXT : "+e.path);continue;}
  }
  QSqlQuery done(db_);done.prepare("UPDATE journal_outbox SET done=1 WHERE id=?");done.addBindValue(e.id);if(!done.exec())status_->setText("Échec validation journal : "+done.lastError().text());
 }
}
void Window::refresh(){
 auto query=[&](bool saved,QString filter){QString select="SELECT substr(m.timestamp,12,8) AS Heure,substr(m.timestamp,1,10) AS Date,printf('%.6f',m.frequency/1000000.0) AS MHz,m.baud AS Baud,printf('%07d',m.ric) AS Capcode,m.function AS Fonction,m.type AS Type,";if(saved)select+="c.alias AS Alias,";select+="m.text AS Message FROM messages m LEFT JOIN capcodes c ON c.ric=m.ric WHERE 1=1 ";QStringList types;if(allowAlpha_->isChecked())types<<"'ALPHA'";if(allowNumeric_->isChecked())types<<"'NUMERIC'";if(allowTone_->isChecked())types<<"'TONE'";if(allowDiagnostic_->isChecked())types<<"'DIAGNOSTIC'";select+=types.isEmpty()?"AND 0 ":"AND m.type IN ("+types.join(',')+") ";if(saved)select+="AND c.active=1 AND c.show_bottom=1 ";if(!filter.isEmpty()){filter.replace("'","''");select+="AND (m.text LIKE '%"+filter+"%' OR printf('%07d',m.ric) LIKE '%"+filter+"%' OR c.alias LIKE '%"+filter+"%') ";}return select+"ORDER BY m.id DESC";};
 QHash<QString,QColor> colors;QSqlQuery colorQuery("SELECT printf('%07d',ric),color FROM capcodes",db_);while(colorQuery.next())colors.insert(colorQuery.value(0).toString(),QColor(colorQuery.value(1).toString()));
 static_cast<MessageModel*>(all_)->colors=colors;static_cast<MessageModel*>(saved_)->colors=colors;
 static_cast<MessageModel*>(all_)->apply(query(false,search_->text()),db_);static_cast<MessageModel*>(saved_)->apply(query(true,savedSearch_->text()),db_);QSqlQuery q("SELECT count(*) FROM messages WHERE type IN ('ALPHA','NUMERIC')",db_);q.next();count_->setText("Messages texte enregistrés : "+q.value(0).toString());if(autoscroll_){allView_->scrollToTop();savedView_->scrollToTop();}
}
void Window::copyTable(QTableView* view){QStringList lines;for(const auto& idx:view->selectionModel()->selectedRows()){QStringList cells;for(int c=0;c<view->model()->columnCount();++c)cells<<view->model()->index(idx.row(),c).data().toString();lines<<cells.join('\t');}QApplication::clipboard()->setText(lines.join('\n'));}
void Window::exportTable(QTableView* view){
 auto path=QFileDialog::getSaveFileName(this,"Exporter les résultats",{},"CSV (*.csv)");if(path.isEmpty())return;auto model=qobject_cast<QSqlQueryModel*>(view->model());while(model->canFetchMore())model->fetchMore();QSaveFile f(path);if(!f.open(QIODevice::WriteOnly)){failure(f.errorString());return;}QTextStream out(&f);out.setEncoding(QStringConverter::Utf8);for(int r=-1;r<model->rowCount();++r){QStringList cells;for(int c=0;c<model->columnCount();++c){QString v=r<0?model->headerData(c,Qt::Horizontal).toString():model->index(r,c).data().toString();v.replace('"',"\"\"");cells<<"\""+v+"\"";}out<<cells.join(';')<<'\n';}out.flush();if(!f.commit())failure(f.errorString());
}


