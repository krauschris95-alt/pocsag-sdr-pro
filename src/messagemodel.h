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
// Virtual rows, bounded page cache. No QSqlQuery survives a read operation.
class MessageModel:public QSqlQueryModel {
public:
 explicit MessageModel(int windowRows=1000000):windowRows_(qMax(256,windowRows)){}
 QHash<QString,QColor> colors;std::function<void(QString)> problem;
 int rowCount(const QModelIndex& parent={})const override{return parent.isValid()?0:total_;}
 int columnCount(const QModelIndex& parent={})const override{return parent.isValid()?0:headers_.size();}
 QModelIndex index(int row,int column,const QModelIndex& parent={})const override{return !parent.isValid()&&row>=0&&row<total_&&column>=0&&column<headers_.size()?createIndex(row,column):QModelIndex{};}
 QModelIndex parent(const QModelIndex&)const override{return {};}
 bool canFetchMore(const QModelIndex& ={})const override{return false;}
 void fetchMore(const QModelIndex& ={})override{}
 QVariant headerData(int section,Qt::Orientation orientation,int role=Qt::DisplayRole)const override{if(role==Qt::DisplayRole&&orientation==Qt::Horizontal)return headers_.value(section);return {};}
 QVariant data(const QModelIndex& idx,int role=Qt::DisplayRole)const override{
  if(!idx.isValid())return {};if(role==Qt::FontRole&&idx.column()==0){QFont font("Consolas",10);font.setBold(true);return font;}
  if(role==Qt::ForegroundRole){if(idx.column()==1||idx.column()==2||(headers_.size()==8&&idx.column()==6))return QColor("#c48aff");if(idx.column()==3||idx.column()==5)return QColor("#ffe56b");if(idx.column()==0||(headers_.size()==8&&idx.column()==6)){auto color=colors.value(data(index(idx.row(),0),Qt::DisplayRole).toString());if(color.isValid())return color;}return QColor("#42e6ff");}
  if(role!=Qt::DisplayRole&&role!=Qt::ToolTipRole&&role!=Qt::UserRole)return {};int page=idx.row()/PageSize;if(!pages_.contains(page)&&!load(page))return {};auto row=pages_.value(page).value(idx.row()%PageSize);return role==Qt::UserRole?row.value(headers_.size()):row.value(idx.column());
 }
 // A bounded visible window is not a retention limit: navigate across all 64-bit rows.
 qint64 totalRows()const{return totalRows_;}qint64 firstRow()const{return first_;}
 void showOlder(){move(qMax<qint64>(0,first_-windowRows_));}void showNewer(){move(qMin(qMax<qint64>(0,totalRows_-windowRows_),first_+windowRows_));}void showFirst(){move(0);}void showLast(){move(qMax<qint64>(0,totalRows_-windowRows_));}
 bool apply(QString query,const QSqlDatabase& db,bool followTail=true){bool changed=query!=base_;db_=db;QString body=query.left(query.lastIndexOf("ORDER BY"));QSqlQuery count(db_);if(!count.exec("SELECT count(*) FROM ("+body+")")||!count.next()){if(problem)problem(count.lastError().text());return false;}qint64 actual=count.value(0).toLongLong();count.finish();qint64 first=first_;if(changed||(followTail&&column_<0))first=qMax<qint64>(0,actual-windowRows_);first=qMin(first,qMax<qint64>(0,actual-windowRows_));int visible=int(qMin<qint64>(windowRows_,actual-first));bool append=!changed&&first==first_&&column_<0&&visible>=total_&&!headers_.isEmpty();totalRows_=actual;
  if(append){if(visible>total_){beginInsertRows({},total_,visible-1);total_=visible;pages_.clear();order_.clear();endInsertRows();}else{pages_.clear();order_.clear();if(total_)emit dataChanged(index(0,0),index(total_-1,headers_.size()-1));}}
  else{beginResetModel();base_=query;first_=first;total_=visible;pages_.clear();order_.clear();QSqlQuery shape(db_);if(shape.exec(body+" LIMIT 0")){headers_.clear();auto record=shape.record();for(int i=0;i<record.count();++i)headers_<<record.fieldName(i);}shape.finish();endResetModel();}return true;
 }
 void sort(int column,Qt::SortOrder order)override{column_=column;sortOrder_=order;if(base_.isEmpty())return;move(column<0?qMax<qint64>(0,totalRows_-windowRows_):0);}
 int cachedRows()const{int n=0;for(auto& p:pages_)n+=p.size();return n;}
private:
 int windowRows_;qint64 totalRows_=0,first_=0;void move(qint64 first){beginResetModel();first_=first;total_=int(qMin<qint64>(windowRows_,totalRows_-first_));pages_.clear();order_.clear();endResetModel();}
 static constexpr int PageSize=256,CachePages=16;QString base_;QSqlDatabase db_;QStringList headers_;int total_=0,column_=-1;Qt::SortOrder sortOrder_=Qt::AscendingOrder;
 mutable QHash<int,QVector<QVariantList>> pages_;mutable QList<int> order_;
 bool load(int page)const{QString body=base_.left(base_.lastIndexOf("ORDER BY"));QString order=column_>=0?QString::number(column_+1)+(sortOrder_==Qt::AscendingOrder?" ASC":" DESC"):"m.id ASC";qint64 offset=first_+qint64(page)*PageSize;int n=int(qMin<qint64>(PageSize,totalRows_-offset));bool tail=column_<0&&offset>totalRows_/2;body.replace(" FROM messages m",",m.id AS _event_id FROM messages m");QString sql=body+"ORDER BY "+(tail?QString("m.id DESC"):order)+QString(" LIMIT %1 OFFSET %2").arg(n).arg(tail?totalRows_-offset-n:offset);QSqlQuery q(db_);q.setForwardOnly(true);if(!q.exec(sql)){if(problem)problem(q.lastError().text());return false;}QVector<QVariantList> rows;while(q.next()){QVariantList row;for(int c=0;c<=headers_.size();++c)row<<q.value(c);rows<<row;}bool ok=!q.lastError().isValid();auto error=q.lastError().text();q.finish();if(!ok){if(problem)problem(error);return false;}if(tail)std::reverse(rows.begin(),rows.end());while(order_.size()>=CachePages)pages_.remove(order_.takeFirst());pages_.insert(page,rows);order_.append(page);return true;}
};
