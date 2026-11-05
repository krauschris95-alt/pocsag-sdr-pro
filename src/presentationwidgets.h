#pragma once
#include <QtWidgets>
// Preserve the complete value for accessibility and tooltips while painting an
// ellipsis in narrow status-bar zones instead of cutting a path or an error.
class ElidedLabel:public QLabel {
public:using QLabel::QLabel;
protected:void paintEvent(QPaintEvent*)override{QStyleOption option;option.initFrom(this);QPainter painter(this);style()->drawPrimitive(QStyle::PE_Widget,&option,&painter,this);painter.setPen(palette().color(QPalette::WindowText));painter.setFont(font());painter.drawText(contentsRect(),Qt::AlignLeft|Qt::AlignVCenter,fontMetrics().elidedText(text(),Qt::ElideMiddle,contentsRect().width()));}
};
