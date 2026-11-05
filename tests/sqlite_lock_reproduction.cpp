#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QTemporaryDir>
#include <iostream>
int main(int argc,char**argv){QCoreApplication app(argc,argv);QTemporaryDir dir;auto gui=QSqlDatabase::addDatabase("QSQLITE","gui");gui.setDatabaseName(dir.path()+"/test.db");gui.open();QSqlQuery q(gui);q.exec("PRAGMA journal_mode=WAL");q.exec("CREATE TABLE messages(id INTEGER PRIMARY KEY, text TEXT)");q.exec("WITH RECURSIVE n(x) AS(VALUES(1) UNION ALL SELECT x+1 FROM n WHERE x<1000) INSERT INTO messages(text) SELECT 'test' FROM n");q.finish();QSqlQueryModel model;model.setQuery("SELECT * FROM messages ORDER BY id DESC",gui);std::cout<<"Rows fetched in persistent cursor: "<<model.rowCount()<<'\n';auto txt=QSqlDatabase::addDatabase("QSQLITE","txt");txt.setDatabaseName(gui.databaseName());txt.open();QSqlQuery w(txt);bool updated=w.exec("INSERT INTO messages(text) VALUES('worker')");gui.transaction();bool inserted=q.exec("INSERT INTO messages(text) VALUES('received')");std::cout<<"TXT writer="<<updated<<" receiving insert="<<inserted<<" error="<<q.lastError().text().toStdString()<<" native="<<q.lastError().nativeErrorCode().toStdString()<<'\n';gui.rollback();model.clear();bool recovered=q.exec("INSERT INTO messages(text) VALUES('after cursor release')");std::cout<<"After cursor released: "<<recovered<<'\n';return !updated||inserted||!recovered;}
