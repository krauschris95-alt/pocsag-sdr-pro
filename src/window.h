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
#include <complex>
#include <memory>
class Spectrum:public QWidget {
public:
 explicit Spectrum(QWidget* p=nullptr):QWidget(p){setMinimumHeight(160);}
 void updateIq(const QByteArray& b);
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
 void sql(const QString& text);void refresh();void received(pocsag::Message m,quint32 frequency=0,QString receiver={});void start(QString file={});
 void capcodes();void journal();void exportTable(QTableView* view);void copyTable(QTableView* view);void failure(QString message);
 void flushJournal();
};
