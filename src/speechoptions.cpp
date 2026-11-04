#include "window.h"
#include <QtWidgets>
#include <QSettings>
#include <QSqlQuery>
void Window::speechOptions(){
 QDialog dialog(this);dialog.setWindowTitle("Synthèse vocale Windows — hors connexion");auto form=new QFormLayout(&dialog);
 auto enabled=new QCheckBox("Lecture vocale générale ON");enabled->setChecked(speechSettings_.enabled);form->addRow(enabled);
 auto voice=new QComboBox;voice->addItem("Voix Windows par défaut","");auto voices=SpeechService::voices();for(auto& v:voices)voice->addItem(v.name+(v.french?" (français)":""),v.id);voice->setCurrentIndex(qMax(0,voice->findData(speechSettings_.voice)));form->addRow("Voix installée",voice);
 auto output=new QComboBox;output->addItem("Périphérique audio par défaut","");for(auto& v:SpeechService::outputs())output->addItem(v.name,v.id);output->setCurrentIndex(qMax(0,output->findData(speechSettings_.output)));form->addRow("Sortie SAPI",output);
 auto volume=new QSpinBox;volume->setRange(0,100);volume->setValue(speechSettings_.volume);auto rate=new QSpinBox;rate->setRange(-10,10);rate->setValue(speechSettings_.rate);auto limit=new QSpinBox;limit->setRange(1,10000);limit->setValue(speechSettings_.limit);auto policy=new QComboBox;policy->addItems({"Refuser le nouveau message","Retirer le plus ancien en attente"});policy->setCurrentIndex(speechSettings_.dropOldest?1:0);form->addRow("Volume",volume);form->addRow("Vitesse (-10 à +10)",rate);form->addRow("Limite de la file",limit);form->addRow("File pleine",policy);
 auto read=[&]{return SpeechSettings{enabled->isChecked(),voice->currentData().toString(),output->currentData().toString(),volume->value(),rate->value(),limit->value(),policy->currentIndex()==1};};
 auto test=new QPushButton("Test vocal");auto stop=new QPushButton("Arrêter la voix et vider la file");form->addRow(test);form->addRow(stop);connect(test,&QPushButton::clicked,&dialog,[&]{auto settings=read();settings.enabled=true;speech_.configure(settings);speech_.enqueue({QUuid::createUuid().toString(),"POCSAG SDR Pro","Test de lecture vocale Windows. Le récepteur reste indépendant."});});connect(stop,&QPushButton::clicked,&speech_,&SpeechService::stopSpeaking);
 auto buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);form->addRow(buttons);connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]{speechSettings_=read();speech_.configure(speechSettings_);storePreferences();dialog.accept();});connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);dialog.exec();speech_.configure(speechSettings_);
}
void Window::replay(QTableView* view){
 auto rows=view->selectionModel()->selectedRows();if(rows.isEmpty())return;auto model=view->model();int row=rows.first().row();auto ric=model->index(row,0).data().toUInt();QString alias;QSqlQuery q(db_);q.prepare("SELECT alias FROM capcodes WHERE ric=?");q.addBindValue(ric);q.exec();if(q.next())alias=q.value(0).toString();if(alias.isEmpty())alias=QString("Capcode %1").arg(ric,7,10,QChar('0'));if(!speechSettings_.enabled){status_->setText("Activer la lecture vocale générale dans Outils");return;}speech_.enqueue({"manual-"+QUuid::createUuid().toString(),alias,model->index(row,model->columnCount()-1).data().toString()});
}
void Window::technical(QString message){
 status_->setText(message);QString path=QFileInfo(db_.databaseName()).absolutePath()+"/technical.log";QFile file(path);if(file.size()>2*1024*1024){QFile::remove(path+".1");file.rename(path+".1");file.setFileName(path);}if(file.open(QIODevice::WriteOnly|QIODevice::Append))file.write((QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)+" "+message+"\r\n").toUtf8());
}
QStringList Window::disabledTypes()const{QStringList list;if(!allowTone_->isChecked())list<<"TONE";if(!allowDiagnostic_->isChecked())list<<"DIAGNOSTIC";return list;}
