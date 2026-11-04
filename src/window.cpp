#include "window.h"
#include <QtWidgets>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSettings>
#include <QStandardPaths>
#include <QSaveFile>
#include <cmath>
#include <numbers>
#include <set>
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
 if(levels_.isEmpty()){p.setPen(Qt::lightGray);p.drawText(rect(),Qt::AlignCenter,"Aucune donnée IQ — spectre en attente");return;}
 QPainterPath line;for(int i=0;i<levels_.size();++i){double x=double(i)*width()/levels_.size(),y=std::clamp((-levels_[i]-20)/100.f,0.f,1.f)*h;if(!i)line.moveTo(x,y);else line.lineTo(x,y);}p.setPen(QColor("#42dcff"));p.drawPath(line);p.drawImage(QRect(0,h,width(),height()-h),waterfall_);
}
void Window::sql(const QString& text){QSqlQuery q(db_);if(!q.exec(text))throw std::runtime_error(q.lastError().text().toStdString());}
static QTableView* table(QSqlQueryModel* model){auto t=new QTableView;t->setModel(model);t->setSelectionBehavior(QAbstractItemView::SelectRows);t->setSelectionMode(QAbstractItemView::ExtendedSelection);t->setAlternatingRowColors(true);t->verticalHeader()->hide();t->verticalHeader()->setDefaultSectionSize(25);t->horizontalHeader()->setStretchLastSection(true);return t;}
Window::Window(QString storageDirectory){
 setWindowTitle("POCSAG SDR Pro — 0.1.0 (validation en cours)");resize(1440,940);
 auto dir=storageDirectory.isEmpty()?QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation):storageDirectory;QDir().mkpath(dir);
 db_=QSqlDatabase::addDatabase("QSQLITE");db_.setDatabaseName(dir+"/pocsag.db");if(!db_.open())throw std::runtime_error(db_.lastError().text().toStdString());
 sql("PRAGMA journal_mode=WAL");sql("PRAGMA busy_timeout=5000");
 sql("CREATE TABLE IF NOT EXISTS messages(id INTEGER PRIMARY KEY, timestamp TEXT, frequency INTEGER, baud INTEGER, ric INTEGER, function INTEGER, type TEXT, text TEXT, corrected INTEGER, receiver TEXT)");
 sql("CREATE INDEX IF NOT EXISTS messages_ric ON messages(ric)");
 sql("CREATE TABLE IF NOT EXISTS capcodes(ric INTEGER PRIMARY KEY CHECK(ric BETWEEN 0 AND 2097151),alias TEXT,color TEXT DEFAULT '#47d7ff',active INTEGER DEFAULT 1,show_bottom INTEGER DEFAULT 1,path TEXT DEFAULT '',description TEXT DEFAULT '')");
 sql("CREATE TABLE IF NOT EXISTS profiles(name TEXT PRIMARY KEY,path TEXT,capcodes TEXT DEFAULT '',enabled INTEGER DEFAULT 0)");
 all_=new QSqlQueryModel(this);saved_=new QSqlQueryModel(this);
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
 auto viewMenu=menuBar()->addMenu("Affichage");auto help=menuBar()->addMenu("Aide");help->addAction("À propos",this,[this]{QMessageBox::information(this,"POCSAG SDR Pro","Version 0.1.0\nRéception RTL-SDR directe et fichiers IQ CU8.\nAucune validation matérielle à ce stade.\nLe type est déduit de la fonction : 0 numérique, 1–3 alpha.\nHackRF et multicanal non disponibles dans cette version.");});
 auto settings=new QHBoxLayout;layout->addLayout(settings);devices_=new QComboBox;devices_->addItems(Receiver::devices());settings->addWidget(new QLabel("Appareil SDR :"));settings->addWidget(devices_);
 frequency_=new QDoubleSpinBox;frequency_->setRange(24,1766);frequency_->setDecimals(6);frequency_->setValue(146.95);frequency_->setSuffix(" MHz");settings->addWidget(frequency_);
 gain_=new QDoubleSpinBox;gain_->setRange(0,50);gain_->setValue(42);gain_->setSuffix(" dB");settings->addWidget(gain_);ppm_=new QDoubleSpinBox;ppm_->setRange(-200,200);ppm_->setSuffix(" PPM");settings->addWidget(ppm_);
 baud_=new QComboBox;baud_->addItems({"AUTO","512","1200","2400"});settings->addWidget(baud_);settings->addWidget(new QLabel("IQ : 240 kS/s • canal centré"));status_=new QLabel("Réception arrêtée");settings->addWidget(status_,1);
 spectrum_=new Spectrum;layout->addWidget(spectrum_);
 auto split=new QSplitter(Qt::Vertical);layout->addWidget(split,1);
 auto panel=[&](QString title,QSqlQueryModel* model,QTableView*& t,QLineEdit*& search){auto w=new QWidget;auto l=new QVBoxLayout(w);l->addWidget(new QLabel(title));auto bar=new QHBoxLayout;search=new QLineEdit;search->setPlaceholderText("Rechercher message, capcode ou alias");bar->addWidget(search);auto copy=new QPushButton("Copier");auto exp=new QPushButton("Exporter CSV");bar->addWidget(copy);bar->addWidget(exp);l->addLayout(bar);t=table(model);l->addWidget(t);auto tv=t;connect(copy,&QPushButton::clicked,this,[this,tv]{copyTable(tv);});connect(exp,&QPushButton::clicked,this,[this,tv]{exportTable(tv);});connect(search,&QLineEdit::textChanged,this,[this]{refresh();});connect(tv,&QTableView::doubleClicked,this,[this,tv](QModelIndex idx){QMessageBox::information(this,"Message complet",tv->model()->index(idx.row(),tv->model()->columnCount()-1).data().toString());});split->addWidget(w);return w;};
 panel("TOUS LES MESSAGES POCSAG",all_,allView_,search_);auto bottom=panel("MESSAGES DES CAPCODES ENREGISTRÉS",saved_,savedView_,savedSearch_);split->setSizes({420,220});auto visible=viewMenu->addAction("Panneau des capcodes enregistrés");visible->setCheckable(true);visible->setChecked(true);connect(visible,&QAction::toggled,bottom,&QWidget::setVisible);
 auto scroll=viewMenu->addAction("Défilement automatique");scroll->setCheckable(true);scroll->setChecked(true);connect(scroll,&QAction::toggled,this,[this](bool v){autoscroll_=v;});
 auto logs=new QHBoxLayout;layout->addLayout(logs);logging_=new QCheckBox("Enregistrement TXT");generalCopy_=new QCheckBox("Copie journal général");logPath_=new QLineEdit;auto choose=new QPushButton("Choisir un autre fichier…");logs->addWidget(logging_);logs->addWidget(generalCopy_);logs->addWidget(logPath_,1);logs->addWidget(choose);connect(choose,&QPushButton::clicked,this,[this]{auto f=QFileDialog::getSaveFileName(this,"Journal texte (ajout sans effacement)",logPath_->text(),"Texte (*.txt)");if(!f.isEmpty())logPath_->setText(f);});count_=new QLabel;statusBar()->addPermanentWidget(count_);
 QSettings s;frequency_->setValue(s.value("frequency",146.95).toDouble());gain_->setValue(s.value("gain",42).toDouble());ppm_->setValue(s.value("ppm",0).toDouble());logPath_->setText(s.value("log",dir+"/general.txt").toString());logging_->setChecked(s.value("logging",false).toBool());generalCopy_->setChecked(s.value("generalCopy",true).toBool());restoreGeometry(s.value("geometry").toByteArray());
 connect(&radio_,&Receiver::samples,spectrum_,&Spectrum::updateIq);connect(&radio_,&Receiver::decoded,this,[this](quint32 ric,int function,int baud,QString type,QString text,int corrected,quint32 frequency,QString receiver){received({ric,function,baud,type.toStdString(),text.toStdString(),corrected},frequency,receiver);});connect(&radio_,&Receiver::problem,this,&Window::failure);connect(&radio_,&Receiver::state,status_,&QLabel::setText);refresh();
 setStyleSheet("QWidget{background:#071425;color:#e6eefb;font-family:'Segoe UI';font-size:12px} QToolBar,QMenuBar,QHeaderView::section{background:#122841;padding:6px} QPushButton,QToolButton{background:#193753;border:1px solid #53708b;border-radius:4px;padding:6px} QLineEdit,QComboBox,QDoubleSpinBox{background:#0b1e33;border:1px solid #4c647d;padding:5px} QTableView{alternate-background-color:#102238;selection-background-color:#005dc8;gridline-color:#193047} QLabel{padding:3px} QSplitter::handle{background:#285074;height:5px}");
}
Window::~Window(){radio_.stop();QSettings s;s.setValue("frequency",frequency_->value());s.setValue("gain",gain_->value());s.setValue("ppm",ppm_->value());s.setValue("log",logPath_->text());s.setValue("logging",logging_->isChecked());s.setValue("generalCopy",generalCopy_->isChecked());s.setValue("geometry",saveGeometry());}
void Window::failure(QString message){status_->setText(message);QMessageBox::warning(this,"Erreur",message);}
void Window::start(QString file){
 radio_.stop();selectedBaud_=baud_->currentText().toInt();
 radio_.config={quint32(frequency_->value()*1e6),240000,int(gain_->value()*10),int(ppm_->value()),devices_->currentIndex(),file,selectedBaud_};radio_.start();
}
void Window::received(pocsag::Message m,quint32 frequency,QString receiver){
 if(!frequency)frequency=radio_.config.frequency;if(receiver.isEmpty())receiver=radio_.config.file.isEmpty()?devices_->currentText():radio_.config.file;
 QSqlQuery q(db_);q.prepare("INSERT INTO messages(timestamp,frequency,baud,ric,function,type,text,corrected,receiver) VALUES(?,?,?,?,?,?,?,?,?)");auto now=QDateTime::currentDateTime().toString(Qt::ISODateWithMs);q.addBindValue(now);q.addBindValue(frequency);q.addBindValue(m.baud);q.addBindValue(m.ric);q.addBindValue(m.function);q.addBindValue(QString::fromStdString(m.type));q.addBindValue(QString::fromStdString(m.text));q.addBindValue(m.corrected);q.addBindValue(receiver);if(!q.exec()){failure(q.lastError().text());return;}
 if(logging_->isChecked()){
  std::set<QString> paths;QSqlQuery cap(db_);cap.prepare("SELECT path FROM capcodes WHERE ric=? AND active=1");cap.addBindValue(m.ric);cap.exec();if(cap.next()&&!cap.value(0).toString().isEmpty())paths.insert(QFileInfo(cap.value(0).toString()).absoluteFilePath());
  if(generalCopy_->isChecked()||paths.empty())if(!logPath_->text().isEmpty())paths.insert(QFileInfo(logPath_->text()).absoluteFilePath());
  QSqlQuery profiles("SELECT path,capcodes FROM profiles WHERE enabled=1",db_);while(profiles.next()){auto rics=profiles.value(1).toString().split(',',Qt::SkipEmptyParts);bool match=rics.isEmpty();for(const auto& r:rics)if(r.trimmed().toUInt()==m.ric)match=true;if(match&&!profiles.value(0).toString().isEmpty())paths.insert(QFileInfo(profiles.value(0).toString()).absoluteFilePath());}
  auto line=QString("%1 | %2 MHz | %3 | %4 | F%5 | %6 | %7\r\n").arg(now).arg(frequency/1e6,0,'f',6).arg(m.baud).arg(m.ric,7,10,QChar('0')).arg(m.function).arg(QString::fromStdString(m.type),QString::fromStdString(m.text).replace('\n',"\\n").replace('\r',"\\r")).toUtf8();
  for(auto& path:paths){QFile f(path);if(!f.open(QIODevice::WriteOnly|QIODevice::Append)||f.write(line)!=line.size()||!f.flush()){status_->setText("Échec journal TXT : "+path+" — message conservé dans SQLite");}}
 }refresh();
}
void Window::refresh(){
 auto query=[&](bool saved,QString filter){QString select="SELECT substr(m.timestamp,12,8) AS Heure,substr(m.timestamp,1,10) AS Date,printf('%.6f',m.frequency/1000000.0) AS MHz,m.baud AS Baud,printf('%07d',m.ric) AS Capcode,m.function AS Fonction,m.type AS Type,";if(saved)select+="c.alias AS Alias,";select+="m.text AS Message FROM messages m LEFT JOIN capcodes c ON c.ric=m.ric WHERE 1=1 ";if(saved)select+="AND c.active=1 AND c.show_bottom=1 ";if(!filter.isEmpty()){filter.replace("'","''");select+="AND (m.text LIKE '%"+filter+"%' OR printf('%07d',m.ric) LIKE '%"+filter+"%' OR c.alias LIKE '%"+filter+"%') ";}return select+"ORDER BY m.id ASC";};
 all_->setQuery(query(false,search_->text()),db_);saved_->setQuery(query(true,savedSearch_->text()),db_);QSqlQuery q("SELECT count(*) FROM messages",db_);q.next();count_->setText("Messages enregistrés : "+q.value(0).toString());if(autoscroll_){allView_->scrollToBottom();savedView_->scrollToBottom();}
}
void Window::copyTable(QTableView* view){QStringList lines;for(const auto& idx:view->selectionModel()->selectedRows()){QStringList cells;for(int c=0;c<view->model()->columnCount();++c)cells<<view->model()->index(idx.row(),c).data().toString();lines<<cells.join('\t');}QApplication::clipboard()->setText(lines.join('\n'));}
void Window::exportTable(QTableView* view){
 auto path=QFileDialog::getSaveFileName(this,"Exporter les résultats",{},"CSV (*.csv)");if(path.isEmpty())return;auto model=qobject_cast<QSqlQueryModel*>(view->model());while(model->canFetchMore())model->fetchMore();QSaveFile f(path);if(!f.open(QIODevice::WriteOnly)){failure(f.errorString());return;}QTextStream out(&f);out.setEncoding(QStringConverter::Utf8);for(int r=-1;r<model->rowCount();++r){QStringList cells;for(int c=0;c<model->columnCount();++c){QString v=r<0?model->headerData(c,Qt::Horizontal).toString():model->index(r,c).data().toString();v.replace('"',"\"\"");cells<<"\""+v+"\"";}out<<cells.join(';')<<'\n';}out.flush();if(!f.commit())failure(f.errorString());
}
void Window::capcodes(){
 QDialog d(this);d.setWindowTitle("GESTION DES CAPCODES ENREGISTRÉS");d.resize(1100,580);auto l=new QVBoxLayout(&d);auto info=new QLabel("Capcode 0 à 2097151 • couleur au format #RRGGBB • journal TXT en ajout UTF-8");l->addWidget(info);
 auto t=new QTableWidget(0,8);t->setHorizontalHeaderLabels({"Capcode","Alias","Couleur","Actif","Afficher en bas","Fichier texte associé","Description",""});t->horizontalHeader()->setStretchLastSection(true);l->addWidget(t);
 auto row=[&](QStringList values){int r=t->rowCount();t->insertRow(r);for(int c=0;c<7;++c){auto item=new QTableWidgetItem(values.value(c));if(c==3||c==4){item->setCheckState(values.value(c).toInt()?Qt::Checked:Qt::Unchecked);}t->setItem(r,c,item);}auto browse=new QPushButton("…");t->setCellWidget(r,7,browse);connect(browse,&QPushButton::clicked,&d,[&,r]{auto f=QFileDialog::getSaveFileName(&d,"Journal associé",{},"TXT (*.txt)");if(!f.isEmpty())t->item(r,5)->setText(f);});};
 QSqlQuery q("SELECT ric,alias,color,active,show_bottom,path,description FROM capcodes ORDER BY ric",db_);while(q.next()){QStringList v;v<<QString("%1").arg(q.value(0).toUInt(),7,10,QChar('0'));for(int c=1;c<7;++c)v<<q.value(c).toString();row(v);}
 auto bar=new QHBoxLayout;l->addLayout(bar);auto add=new QPushButton("Ajouter");auto del=new QPushButton("Supprimer");auto imp=new QPushButton("Importer JSON");auto exp=new QPushButton("Exporter JSON");bar->addWidget(add);bar->addWidget(del);bar->addWidget(imp);bar->addWidget(exp);connect(add,&QPushButton::clicked,&d,[&]{row({"0000000","","#47d7ff","1","1","",""});});connect(del,&QPushButton::clicked,&d,[&]{if(t->currentRow()>=0)t->removeRow(t->currentRow());});
 connect(exp,&QPushButton::clicked,&d,[&]{auto f=QFileDialog::getSaveFileName(&d,"Exporter capcodes",{},"JSON (*.json)");if(f.isEmpty())return;QJsonArray arr;for(int r=0;r<t->rowCount();++r){QJsonArray a;for(int c=0;c<7;++c)a.append(c==3||c==4?QString::number(t->item(r,c)->checkState()==Qt::Checked):t->item(r,c)->text());arr.append(a);}QSaveFile file(f);if(file.open(QIODevice::WriteOnly)){file.write(QJsonDocument(arr).toJson());file.commit();}});
 connect(imp,&QPushButton::clicked,&d,[&]{auto f=QFileDialog::getOpenFileName(&d,"Importer capcodes",{},"JSON (*.json)");QFile file(f);if(!file.open(QIODevice::ReadOnly))return;QJsonParseError err;auto doc=QJsonDocument::fromJson(file.readAll(),&err);if(err.error!=QJsonParseError::NoError||!doc.isArray()){QMessageBox::warning(&d,"Import","JSON invalide");return;}for(auto a:doc.array()){QStringList v;for(auto x:a.toArray())v<<x.toString();if(v.size()==7)row(v);}});
 auto buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);l->addWidget(buttons);connect(buttons,&QDialogButtonBox::rejected,&d,&QDialog::reject);connect(buttons,&QDialogButtonBox::accepted,&d,[&]{
  std::set<uint> ids;for(int r=0;r<t->rowCount();++r){bool ok;auto ric=t->item(r,0)->text().toUInt(&ok);if(!ok||ric>2097151||!ids.insert(ric).second||!QColor(t->item(r,2)->text()).isValid()){QMessageBox::warning(&d,"Validation","Capcode invalide, doublon ou couleur invalide");return;}}
  db_.transaction();QSqlQuery clear(db_);if(!clear.exec("DELETE FROM capcodes")){db_.rollback();return;}for(int r=0;r<t->rowCount();++r){QSqlQuery ins(db_);ins.prepare("INSERT INTO capcodes(ric,alias,color,active,show_bottom,path,description) VALUES(?,?,?,?,?,?,?)");for(int c=0;c<7;++c)ins.addBindValue(c==3||c==4?QVariant(t->item(r,c)->checkState()==Qt::Checked):QVariant(t->item(r,c)->text()));if(!ins.exec()){db_.rollback();QMessageBox::warning(&d,"SQLite",ins.lastError().text());return;}}if(!db_.commit()){db_.rollback();return;}d.accept();refresh();});d.exec();
}
void Window::journal(){
 QDialog d(this);d.setWindowTitle("JOURNAL TEXTE ET PROFILS");d.resize(900,450);auto l=new QVBoxLayout(&d);l->addWidget(new QLabel("Profils actifs : fichier UTF-8 en ajout. Capcodes séparés par virgules ; vide = tous les messages."));auto t=new QTableWidget(0,4);t->setHorizontalHeaderLabels({"Profil","Fichier texte (chemin complet)","Capcodes","Actif"});t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);l->addWidget(t);QSqlQuery q("SELECT name,path,capcodes,enabled FROM profiles",db_);while(q.next()){int r=t->rowCount();t->insertRow(r);for(int c=0;c<4;++c)t->setItem(r,c,new QTableWidgetItem(q.value(c).toString()));}
 auto add=new QPushButton("Ajouter un profil");l->addWidget(add);connect(add,&QPushButton::clicked,&d,[&]{int r=t->rowCount();t->insertRow(r);for(int c=0;c<4;++c)t->setItem(r,c,new QTableWidgetItem(c==0?"Nouveau":c==3?"0":""));});auto del=new QPushButton("Supprimer le profil sélectionné");l->addWidget(del);connect(del,&QPushButton::clicked,&d,[&]{if(t->currentRow()>=0)t->removeRow(t->currentRow());});auto b=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);l->addWidget(b);connect(b,&QDialogButtonBox::rejected,&d,&QDialog::reject);connect(b,&QDialogButtonBox::accepted,&d,[&]{db_.transaction();QSqlQuery clear(db_);clear.exec("DELETE FROM profiles");for(int r=0;r<t->rowCount();++r){QSqlQuery ins(db_);ins.prepare("INSERT INTO profiles(name,path,capcodes,enabled) VALUES(?,?,?,?)");for(int c=0;c<4;++c)ins.addBindValue(t->item(r,c)->text());if(!ins.exec()){db_.rollback();QMessageBox::warning(&d,"Profil invalide",ins.lastError().text());return;}}if(db_.commit())d.accept();});d.exec();
}
