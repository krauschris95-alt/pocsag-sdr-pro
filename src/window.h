#pragma once
#include "decoder.h"
#include "receiver.h"
#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QTableView>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QAction>
#include <QDateTime>
#include <QMenu>
#include <complex>
#include <memory>
class Spectrum:public QWidget {
public:
 explicit Spectrum(QWidget* p=nullptr):QWidget(p){setMinimumHeight(160);}
 void updateIq(const QByteArray& b);
 quint64 frequency=169626000;double zoom=1;
 void tuneFrequency(quint64 value){if(frequency!=value){levels_.clear();waterfall_=QImage();}frequency=value;update();}
protected:void paintEvent(QPaintEvent*)override;
private:QVector<float> levels_;QImage waterfall_;
};
class Window:public QMainWindow {
 Q_OBJECT
public:explicit Window(QString storageDirectory={});~Window();
private:
 friend struct IntegrationChecks;
 QSqlDatabase db_;QSqlQueryModel *all_,*saved_;QTableView *allView_,*savedView_;Receiver radio_;
 QDoubleSpinBox *frequency_,*gain_,*ppm_;QComboBox *devices_,*baud_;QLabel *status_,*count_;QLineEdit *search_,*savedSearch_,*logPath_;QCheckBox *logging_,*generalCopy_;Spectrum* spectrum_;
 int selectedBaud_=0;bool autoscroll_=true;
 QAction *allowAlpha_=nullptr,*allowNumeric_=nullptr,*allowTone_=nullptr,*allowDiagnostic_=nullptr;
 QComboBox *format_=nullptr,*step_=nullptr;
 QCheckBox *daily_=nullptr;QLineEdit *root_=nullptr,*profileName_=nullptr;QLabel* activePath_=nullptr;
 void sql(const QString& text);void refresh();void received(pocsag::Message m,quint32 frequency=0,QString receiver={},QDateTime timestamp={});void start(QString file={});
 void capcodes();void journal();void exportTable(QTableView* view);void copyTable(QTableView* view);void failure(QString message);
 void flushJournal();
 void setupOptions(QAction* start,QAction* stop,QAction* open,QAction* cap,QAction* log,QMenu* view,QMenu* help);
 void saveOptions();void retune();void migrate();
 QString generalJournalPath(QDateTime timestamp)const;
 static QString dailyPath(QString root,QString name,QDateTime timestamp);
 QString settingsFile_;
};
