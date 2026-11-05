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
  if(role!=Qt::DisplayRole&&role!=Qt::ToolTipRole)return {};int page=idx.row()/PageSize;if(!pages_.contains(page)&&!load(page))return {};return pages_.value(page).value(idx.row()%PageSize).value(idx.column());
 }
 bool apply(QString query,const QSqlDatabase& db){bool changed=query!=base_;db_=db;QString body=query.left(query.lastIndexOf("ORDER BY"));QSqlQuery count(db_);if(!count.exec("SELECT count(*) FROM ("+body+")")||!count.next()){if(problem)problem(count.lastError().text());return false;}int total=count.value(0).toInt();count.finish();bool append=!changed&&column_<0&&total>=total_&&headers_.size()>0;
  if(append){if(total>total_){beginInsertRows({},total_,total-1);total_=total;pages_.clear();order_.clear();endInsertRows();}else{pages_.clear();order_.clear();if(total_)emit dataChanged(index(0,0),index(total_-1,headers_.size()-1));}}
  else{beginResetModel();base_=query;total_=total;pages_.clear();order_.clear();QSqlQuery shape(db_);if(shape.exec(body+" LIMIT 0")){headers_.clear();auto record=shape.record();for(int i=0;i<record.count();++i)headers_<<record.fieldName(i);}shape.finish();endResetModel();}return true;
 }
 void sort(int column,Qt::SortOrder order)override{column_=column;sortOrder_=order;if(base_.isEmpty())return;beginResetModel();pages_.clear();order_.clear();endResetModel();}
 int cachedRows()const{int n=0;for(auto& p:pages_)n+=p.size();return n;}
private:
 static constexpr int PageSize=256,CachePages=16;QString base_;QSqlDatabase db_;QStringList headers_;int total_=0,column_=-1;Qt::SortOrder sortOrder_=Qt::AscendingOrder;
 mutable QHash<int,QVector<QVariantList>> pages_;mutable QList<int> order_;
 bool load(int page)const{QString body=base_.left(base_.lastIndexOf("ORDER BY"));QString order=column_>=0?QString::number(column_+1)+(sortOrder_==Qt::AscendingOrder?" ASC":" DESC"):"m.id ASC";int offset=page*PageSize,n=qMin(PageSize,total_-offset);bool tail=column_<0&&offset>total_/2;QString sql=body+"ORDER BY "+(tail?QString("m.id DESC"):order)+QString(" LIMIT %1 OFFSET %2").arg(n).arg(tail?total_-offset-n:offset);QSqlQuery q(db_);q.setForwardOnly(true);if(!q.exec(sql)){if(problem)problem(q.lastError().text());return false;}QVector<QVariantList> rows;while(q.next()){QVariantList row;for(int c=0;c<headers_.size();++c)row<<q.value(c);rows<<row;}bool ok=!q.lastError().isValid();auto error=q.lastError().text();q.finish();if(!ok){if(problem)problem(error);return false;}if(tail)std::reverse(rows.begin(),rows.end());while(order_.size()>=CachePages)pages_.remove(order_.takeFirst());pages_.insert(page,rows);order_.append(page);return true;}
};
