#pragma once
#include "decoder.h"
#include "receiver.h"
#include "speech.h"
#include "pagerworker.h"
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
#include <QProgressBar>
#include <QElapsedTimer>
#include <complex>
#include <memory>
#include <deque>
#include <QJsonObject>
class QSettings;
class Spectrum:public QWidget {
 Q_OBJECT
public:
 explicit Spectrum(QWidget* p=nullptr):QWidget(p){setMinimumHeight(160);}
 void updateIq(const QByteArray& b);
 quint64 frequency=169626000;double zoom=1;
 double sampleRate=240000,bandwidth=21000,top=-20,bottom=-100,contrast=1,brightness=0;
 bool automaticScale=false;int smoothingFrames=4,waterfallStride=1,waterfallFrames=0;
 void tuneFrequency(quint64 value);
 void loadSettings(QSettings& settings);
 void saveSettings(QSettings& settings)const;
 void displaySettingsChanged();
 QPair<double,double> axisRange()const{return automaticScale?qMakePair(autoBottom_,autoTop_):qMakePair(bottom,top);}
 QVector<double> axisTicks()const;
 const QVector<float>& fftLevels()const{return rawLevels_;}
 const QVector<float>& displayedLevels()const{return levels_;}
 QRectF plotRect()const;
 QColor waterfallColor(double db)const;
signals:void tuneRequested(qint64 frequency);
protected:void paintEvent(QPaintEvent*)override;void mousePressEvent(QMouseEvent*)override;
private:QVector<float> levels_,rawLevels_,averagePower_,waterfallDb_;QImage waterfall_;
 double autoBottom_=-100,autoTop_=-20;bool recolor_=true;int historyRows_=0;
 void rebuildWaterfall();
};
class Window:public QMainWindow {
 Q_OBJECT
public:explicit Window(QString storageDirectory={},std::unique_ptr<SpeechBackend> speechBackend={});~Window();
signals:void lowerFilterChanged(bool active);
private:
 friend struct IntegrationChecks;
 QSqlDatabase db_;QSqlQueryModel *all_,*saved_;QTableView *allView_,*savedView_;Receiver radio_;
 QDoubleSpinBox *frequency_,*gain_,*ppm_;QComboBox *devices_,*baud_;QLabel *status_,*count_;QLineEdit *search_,*savedSearch_,*logPath_;QCheckBox *logging_,*generalCopy_;Spectrum* spectrum_;
 int selectedBaud_=0;bool autoscroll_=true;
 QAction *allowAlpha_=nullptr,*allowNumeric_=nullptr,*allowTone_=nullptr,*allowDiagnostic_=nullptr;
 QComboBox *format_=nullptr,*step_=nullptr;
 QCheckBox *daily_=nullptr;QLineEdit *root_=nullptr,*profileName_=nullptr;QLabel* activePath_=nullptr;
 bool glacier_=true;int textSize_=1;QLabel *frequencyMeta_=nullptr,*presentationState_=nullptr;bool eventFilter(QObject* object,QEvent* event)override;void frequencyDialog();bool tuneSelected(quint32 hz,QString* error=nullptr);void resizeEvent(QResizeEvent* event)override;void adaptPresentation();void setupPresentation();void applyPresentation();void updatePresentationState();
 void sql(const QString& text);void refresh();void received(pocsag::Message m,quint32 frequency=0,QString receiver={},QDateTime timestamp={});void start(QString file={});
 void capcodes();void journal();void configureJournals(bool focusPermanent);void txtProfiles();void storageOptions();void exportTable(QTableView* view);void copyTable(QTableView* view);void failure(QString message);
 void flushJournal();
 void setupOptions(QAction* start,QAction* stop,QAction* open,QAction* cap,QAction* log,QMenu* view,QMenu* help);
 void saveOptions();void retune();void migrate();
 QString generalJournalPath(QDateTime timestamp)const;
 static QString dailyPath(QString root,QString name,QDateTime timestamp);
 QString settingsFile_,bootstrapFile_;
 std::unique_ptr<PagerWorker> pagerWorker_;PagerSettings pagerSettings_;void pagerOptions();void startPagerWorker();
 SpeechService speech_;SpeechSettings speechSettings_;std::unique_ptr<JournalWorker> journalWorker_;
 QTabWidget* tabs_=nullptr;QSplitter* messageSplit_=nullptr;QLabel* frequencyDisplay_=nullptr;
 QLineEdit *filterRic_=nullptr,*filterAlias_=nullptr,*filterKeyword_=nullptr;QDoubleSpinBox *filterFrequency_=nullptr;QDateTimeEdit *filterFrom_=nullptr,*filterTo_=nullptr;QComboBox *filterBaud_=nullptr,*filterType_=nullptr;
 bool receiveWanted_=false;bool testStorage_=false;QTimer* refreshTimer_=nullptr;
 void speechOptions();void replay(QTableView* view);void scheduleRefresh();void technical(QString message);void receiverOptions();void finishLayout();
 void reloadDevices();void sdrDevices();void spectrumOptions();void storePreferences();QStringList disabledTypes()const;
 QLabel* sdrIndicator_=nullptr;void updateSdrIndicator();
 std::vector<std::unique_ptr<Receiver>> extraRadios_;Receiver* selectedRadio_=nullptr;QMenu* sourcesMenu_=nullptr;
 void connectReceiver(Receiver* receiver);void selectReceiver(Receiver* receiver);void stopReceivers();void additionalReceivers();
 Receiver* addReceiver(RadioConfig config);void refreshSources();
 std::deque<QJsonObject> pendingMessages_;QString pendingDirectory_;bool draining_=false;QTimer* ingestionTimer_=nullptr;
 QCheckBox* selectedLogging_=nullptr;QLineEdit* selectedLogPath_=nullptr;QLabel* quality_=nullptr;QAction* voiceAction_=nullptr;QProgressBar* qualityBar_=nullptr;quint64 qualityWords_=0,qualityRejected_=0;QElapsedTimer qualityActivity_;
 quint64 acceptedFrames_=0,persistedFrames_=0,sqliteRetries_=0;QString lastStorageError_;
 void initIngestion();void pumpMessages();bool storePending(const QJsonObject& value);void storageProblem(QString text);
 void soapyOptions();
};
