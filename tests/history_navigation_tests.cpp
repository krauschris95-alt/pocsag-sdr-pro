#include "messagemodel.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTemporaryDir>
#include <QTimer>
#include <iostream>
#include <stdexcept>

static void require(bool ok,const char* message){
 if(!ok)throw std::runtime_error(message);
}

static void run(int size){
 QTemporaryDir dir;
 require(dir.isValid(),"temporary directory");
 auto db=QSqlDatabase::addDatabase("QSQLITE",QString::number(size));
 db.setDatabaseName(dir.filePath("history.db"));
 require(db.open(),"open fixture");
 QSqlQuery q(db);
 require(q.exec("PRAGMA journal_mode=WAL"),"WAL");
 require(q.exec("CREATE TABLE messages(id INTEGER PRIMARY KEY,text TEXT)"),"schema");
 require(q.exec(QString("WITH RECURSIVE n(x) AS (VALUES(1) UNION ALL SELECT x+1 FROM n WHERE x<%1) INSERT INTO messages SELECT x,printf('message-%07d',x) FROM n").arg(size)),"insert history");
 MessageModel model;
 const QString query="SELECT m.id,m.text FROM messages m WHERE 1=1 ORDER BY m.id ASC";
 require(model.apply(query,db),"initial count");
 model.sort(1,Qt::DescendingOrder);
 int row=size/2;
 model.painting=true;
 QEventLoop loop;
 model.onPageReady=[&]{loop.quit();};
 QElapsedTimer time;
 time.start();
 require(!model.index(row,1).data().isValid(),"paint must return before the SQL page completes");
 auto paintMs=time.elapsed();
 require(paintMs<200,"paint stalled");
 QTimer timeout;
 timeout.setSingleShot(true);
 QObject::connect(&timeout,&QTimer::timeout,&loop,&QEventLoop::quit);
 timeout.start(30000);
 loop.exec();
 require(timeout.isActive(),"sorted page timeout");
 require(model.index(row,0).data().toInt()==size-row,"sorted page identity");
 model.painting=false;
 model.sort(-1,Qt::AscendingOrder);
 require(model.index(0,0).data().toInt()==1,"oldest row");
 require(q.exec("INSERT INTO messages(text) VALUES('new')"),"append");
 require(model.apply(query,db,false,true),"incremental refresh");
 require(model.firstRow()==0&&model.totalRows()==size+1&&model.cachedRows()>=256,"preserve older window and cache");
 model.showLast();
 require(model.index(model.rowCount()-1,1).data().toString()=="new","last message");
 require(q.exec("PRAGMA integrity_check")&&q.next()&&q.value(0).toString()=="ok","integrity");
 std::cout<<"history_rows="<<size<<" sorted_paint_ms="<<paintMs<<std::endl;
}

int main(int argc,char** argv){
 QApplication app(argc,argv);
 try{run(100000);run(1000000);}
 catch(const std::exception& error){std::cerr<<error.what()<<std::endl;return 1;}
 return 0;
}