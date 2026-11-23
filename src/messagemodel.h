#pragma once
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <algorithm>
#include <QColor>
#include <QFont>
#include <QHash>
#include <functional>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#include <QUuid>
// Virtual rows, bounded page cache. No QSqlQuery survives a read operation.
class MessageModel:public QSqlQueryModel {
public:
 explicit MessageModel(int windowRows=1000000):windowRows_(qMax(256,windowRows)){}
 bool painting=false;
 bool glacier=true;int pixelSize=15;QHash<QString,QColor> colors;std::function<void(QString)> problem;
 int rowCount(const QModelIndex& parent={})const override{return parent.isValid()?0:total_;}
 int columnCount(const QModelIndex& parent={})const override{return parent.isValid()?0:headers_.size();}
 QModelIndex index(int row,int column,const QModelIndex& parent={})const override{return !parent.isValid()&&row>=0&&row<total_&&column>=0&&column<headers_.size()?createIndex(row,column):QModelIndex{};}
 QModelIndex parent(const QModelIndex&)const override{return {};}
 bool canFetchMore(const QModelIndex& ={})const override{return false;}
 void fetchMore(const QModelIndex& ={})override{}
 QVariant headerData(int section,Qt::Orientation orientation,int role=Qt::DisplayRole)const override{if(role==Qt::DisplayRole&&orientation==Qt::Horizontal)return headers_.value(section);return {};}
 QVariant data(const QModelIndex& idx,int role=Qt::DisplayRole)const override{
  if(!idx.isValid())return {};if(role==Qt::FontRole&&idx.column()==0){QFont font(glacier?"Segoe UI":"Consolas");font.setPixelSize(pixelSize);font.setBold(true);return font;}
  if(role==Qt::ForegroundRole&&glacier){return QColor(idx.column()==0?"#075B91":idx.column()==1||idx.column()==2?"#354D82":"#172C3E");}
  if(role==Qt::ForegroundRole){if(idx.column()==1||idx.column()==2||(headers_.size()==8&&idx.column()==6))return QColor("#c48aff");if(idx.column()==3||idx.column()==5)return QColor("#ffe56b");if(idx.column()==0||(headers_.size()==8&&idx.column()==6)){auto color=colors.value(data(index(idx.row(),0),Qt::DisplayRole).toString());if(color.isValid())return color;}return QColor("#42e6ff");}
  if(role!=Qt::DisplayRole&&role!=Qt::ToolTipRole&&role!=Qt::UserRole)return {};int page=idx.row()/PageSize;auto it=pages_.constFind(page);
  if(it==pages_.cend()){
   auto self=const_cast<MessageModel*>(this);
   if(painting){self->requestPage(page);return {};}
   auto result=readPage(db_,pageQuery(page));
   if(!result.error.isEmpty()){if(problem)problem(result.error);return {};}
   self->cachePage(page,std::move(result.rows));
   it=pages_.constFind(page);
  }
  int offset=idx.row()%PageSize;
  if(offset>=it.value().size())return {};
  const auto& row=it.value().at(offset);return role==Qt::UserRole?row.value(headers_.size()):row.value(idx.column());
 }
 // A bounded visible window is not a retention limit: navigate across all 64-bit rows.
 qint64 totalRows()const{return totalRows_;}qint64 firstRow()const{return first_;}
 void showOlder(){move(qMax<qint64>(0,first_-windowRows_));}void showNewer(){move(qMin(qMax<qint64>(0,totalRows_-windowRows_),first_+windowRows_));}void showFirst(){move(0);}void showLast(){move(qMax<qint64>(0,totalRows_-windowRows_));}
 struct Snapshot{QVariant newest;qint64 rows=0;QString error;};
 static Snapshot countRows(QSqlDatabase db,const QString& body,bool incremental,qint64 lastId){
  Snapshot result;
  if(!db.transaction()){result.error=db.lastError().text();return result;}
  QSqlQuery q(db);
  if(q.exec("SELECT max(id) FROM messages")&&q.next()){
   result.newest=q.value(0);
   q.finish();
   QString filtered=body;
   if(incremental)filtered+=" AND m.id > ?";
   if(q.prepare("SELECT count(*) FROM ("+filtered+")")){
    if(incremental)q.addBindValue(lastId);
    if(q.exec()&&q.next())result.rows=q.value(0).toLongLong();
   }
  }
  if(q.lastError().isValid())result.error=q.lastError().text();
  q.finish();
  if(!db.commit()&&result.error.isEmpty())result.error=db.lastError().text();
  return result;
 }
 bool apply(QString query,const QSqlDatabase& db,bool followTail=true,bool insertsOnly=false){
  db_=db;
  bool incremental=insertsOnly&&query==base_&&hasWatermark_;
  auto body=query.left(query.lastIndexOf("ORDER BY"));
  return accept(query,countRows(db,body,incremental,lastId_),followTail,incremental);
 }
 void applyAsync(QString query,const QSqlDatabase& db,bool insertsOnly,std::function<bool()> follow,std::function<void()> completed){
  if(counting_){
   if(!insertsOnly||query!=requestedQuery_)++revision_;
   bool full=pendingFull_||!insertsOnly;
   pendingFull_=full;
   pending_=[this,query,db,full,follow,completed]{applyAsync(query,db,!full,follow,completed);};
   return;
  }
  counting_=true;
  requestedQuery_=query;
  db_=db;
  bool incremental=insertsOnly&&query==base_&&hasWatermark_;
  auto watcher=new QFutureWatcher<Snapshot>(this);
  connect(watcher,&QFutureWatcher<Snapshot>::finished,this,[this,watcher,query,incremental,follow,completed,rev=revision_]{
   auto result=watcher->result();
   watcher->deleteLater();
   counting_=false;
   if(rev==revision_&&accept(query,result,follow(),incremental))completed();
   auto next=std::move(pending_);
   pending_={};
   pendingFull_=false;
   if(next)next();
  });
  auto body=query.left(query.lastIndexOf("ORDER BY"));
  watcher->setFuture(QtConcurrent::run([file=db.databaseName(),body,incremental,last=lastId_]{
   return readOnly<Snapshot>(file,[&](QSqlDatabase db){return countRows(db,body,incremental,last);});
  }));
 }
private:
 bool counting_=false,pendingFull_=false;
 quint64 revision_=0;
 QString requestedQuery_;
 std::function<void()> pending_;
 template<class Result,class Read>
 static Result readOnly(const QString& file,Read read){
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
 bool accept(const QString& query,const Snapshot& snapshot,bool followTail,bool incremental){
  if(!snapshot.error.isEmpty()){hasWatermark_=false;if(problem)problem(snapshot.error);return false;}
  const bool changed=query!=base_;
  const QString body=query.left(query.lastIndexOf("ORDER BY"));
  const qint64 actual=(incremental?totalRows_:0)+snapshot.rows;
  lastId_=snapshot.newest.toLongLong();
  hasWatermark_=!snapshot.newest.isNull();
  qint64 first=first_;if(changed||(followTail&&column_<0))first=qMax<qint64>(0,actual-windowRows_);first=qMin(first,qMax<qint64>(0,actual-windowRows_));int visible=int(qMin<qint64>(windowRows_,actual-first));bool append=!changed&&first==first_&&column_<0&&visible>=total_&&!headers_.isEmpty();totalRows_=actual;
  if(!incremental||!append||visible!=total_)++generation_;
  if(append){if(visible>total_){beginInsertRows({},total_,visible-1);
    if(incremental){
     // Completed chronological pages retain their position after an append.
     if(total_%PageSize){const int page=total_/PageSize;pages_.remove(page);order_.removeAll(page);}
    }else{pages_.clear();order_.clear();}
    total_=visible;
    endInsertRows();
   }else if(!incremental){pages_.clear();order_.clear();if(total_)emit dataChanged(index(0,0),index(total_-1,headers_.size()-1));}}
  else{beginResetModel();base_=query;first_=first;total_=visible;pages_.clear();order_.clear();QSqlQuery shape(db_);if(shape.exec(body+" LIMIT 0")){headers_.clear();auto record=shape.record();for(int i=0;i<record.count();++i)headers_<<record.fieldName(i);}shape.finish();endResetModel();}return true;
 }
public:
 void sort(int column,Qt::SortOrder order)override{column_=column;sortOrder_=order;if(base_.isEmpty())return;move(column<0?qMax<qint64>(0,totalRows_-windowRows_):0);}
 int cachedRows()const{int n=0;for(auto& p:pages_)n+=p.size();return n;}
private:
 bool hasWatermark_=false;
 qint64 lastId_=0;
 int windowRows_;qint64 totalRows_=0,first_=0;void move(qint64 first){++generation_;beginResetModel();first_=first;total_=int(qMin<qint64>(windowRows_,totalRows_-first_));pages_.clear();order_.clear();endResetModel();}
 static constexpr int PageSize=256,CachePages=16;QString base_;QSqlDatabase db_;QStringList headers_;int total_=0,column_=-1;Qt::SortOrder sortOrder_=Qt::AscendingOrder;
 mutable QHash<int,QVector<QVariantList>> pages_;mutable QList<int> order_;
 struct PageQuery{QString sql;int columns;bool reverse;};
 struct PageResult{QVector<QVariantList> rows;QString error;};
 quint64 generation_=0;

 bool loading_=false;
 PageQuery pageQuery(int page)const{
  QString body=base_.left(base_.lastIndexOf("ORDER BY"));
  QString order=column_>=0?QString::number(column_+1)+(sortOrder_==Qt::AscendingOrder?" ASC":" DESC")+",m.id ASC":"m.id ASC";
  qint64 offset=first_+qint64(page)*PageSize;
  int n=int(qMin<qint64>(PageSize,totalRows_-offset));
  bool tail=column_<0&&offset>totalRows_/2;
  // The worker may open its snapshot after another insert has committed.
  QString source=hasWatermark_?QString("(SELECT * FROM messages WHERE id<=%1)").arg(lastId_):QString("messages");
  body.replace(" FROM messages m",",m.id AS _event_id FROM "+source+" m");
  return {body+"ORDER BY "+(tail?QString("m.id DESC"):order)+QString(" LIMIT %1 OFFSET %2").arg(n).arg(tail?totalRows_-offset-n:offset),int(headers_.size()+1),tail};
 }
 static PageResult readPage(const QSqlDatabase& db,const PageQuery& request){
  PageResult result;
  QSqlQuery q(db);
  q.setForwardOnly(true);
  if(q.exec(request.sql)){
   while(q.next()){
    QVariantList row;
    for(int c=0;c<request.columns;++c)row<<q.value(c);
    result.rows<<std::move(row);
   }
  }
  if(q.lastError().isValid())result.error=q.lastError().text();
  if(request.reverse)std::reverse(result.rows.begin(),result.rows.end());
  return result;
 }
 void cachePage(int page,QVector<QVariantList> rows){
  order_.removeAll(page);
  while(order_.size()>=CachePages)pages_.remove(order_.takeFirst());
  pages_.insert(page,std::move(rows));
  order_.append(page);
 }
 void requestPage(int page){
  if(loading_)return;
  loading_=true;
  auto request=pageQuery(page);
  auto file=db_.databaseName();
  auto watcher=new QFutureWatcher<PageResult>(this);
  connect(watcher,&QFutureWatcher<PageResult>::finished,this,[this,watcher,page,generation=generation_]{
   auto result=watcher->result();
   watcher->deleteLater();
   loading_=false;
   if(generation==generation_){
    if(!result.error.isEmpty()){if(problem)problem(result.error);return;}
    cachePage(page,std::move(result.rows));
   }
   // A fast scroll may have requested a different page while this one was in flight.
   if(onPageReady)onPageReady();
  });
  watcher->setFuture(QtConcurrent::run([file,request]{
   return readOnly<PageResult>(file,[&](const QSqlDatabase& db){return readPage(db,request);});
  }));
 }
public:
 std::function<void()> onPageReady;
};
