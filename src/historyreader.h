#pragma once
#include <QSqlDatabase>
#include <QSqlError>
#include <QUuid>
 template<class Result,class Read>
 Result readHistoryDatabase(const QString& file,Read read){
  auto name=QUuid::createUuid().toString();
  Result result;
  {
   auto db=QSqlDatabase::addDatabase("QSQLITE",name);
   db.setDatabaseName(file);
   db.setConnectOptions("QSQLITE_OPEN_READONLY;QSQLITE_BUSY_TIMEOUT=100");
   if(db.open())result=read(db);
   else result.error=db.lastError().text();
  }
  QSqlDatabase::removeDatabase(name);
  return result;
 }
