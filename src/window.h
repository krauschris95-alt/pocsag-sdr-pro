#pragma once
#include "decoder.h"
#include "receiver.h"
#include "speech.h"
#include "journalworker.h"
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
#include <QTabWidget>
#include <QSplitter>
#include <QDateTimeEdit>
#include <QTimer>
#include <complex>
#include <memory>
class Spectrum:public QWidget {
 Q_OBJECT
public:
 explicit Spectrum(QWidget* p=nullptr):QWidget(p){setMinimumHeight(160);}
 void updateIq(const QByteArray& b);
 quint64 frequency=169626000;double zoom=1;
 double sampleRate=240000,bandwidth=21000,top=-20,bottom=-120,contrast=1;int waterfallStride=1,waterfallFrames=0;
 void tuneFrequency(quint64 value){if(frequency!=value){levels_.clear();waterfall_=QImage();}frequency=value;update();}
signals:void tuneRequested(qint64 frequency);
protected:void paintEvent(QPaintEvent*)override;void mousePressEvent(QMouseEvent*)override;
private:QVector<float> levels_;QImage waterfall_;
};
class Window:public QMainWindow {
 Q_OBJECT
public:explicit Window(QString storageDirectory={},std::unique_ptr<SpeechBackend> speechBackend={});~Window();
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
 SpeechService speech_;SpeechSettings speechSettings_;std::unique_ptr<JournalWorker> journalWorker_;
 QTabWidget* tabs_=nullptr;QSplitter* messageSplit_=nullptr;QLabel* frequencyDisplay_=nullptr;
 QLineEdit *filterRic_=nullptr,*filterAlias_=nullptr,*filterKeyword_=nullptr;QDoubleSpinBox *filterFrequency_=nullptr;QDateTimeEdit *filterFrom_=nullptr,*filterTo_=nullptr;QComboBox *filterBaud_=nullptr,*filterType_=nullptr;
 bool receiveWanted_=false;bool testStorage_=false;QTimer* refreshTimer_=nullptr;
 void speechOptions();void replay(QTableView* view);void scheduleRefresh();void technical(QString message);void receiverOptions();void finishLayout();
 void reloadDevices();void storePreferences();QStringList disabledTypes()const;
 QLabel* sdrIndicator_=nullptr;void updateSdrIndicator();
};
