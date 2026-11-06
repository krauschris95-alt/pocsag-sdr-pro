#include "window.h"
#include <QtWidgets>
#include <QSettings>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <cstring>
namespace {constexpr int bins=1024,rows=160;
double setting(QSettings& s,const char* name,double fallback,double low,double high){bool ok=false;double value=s.value(name,fallback).toDouble(&ok);return ok&&std::isfinite(value)?std::clamp(value,low,high):fallback;}
}
void Spectrum::loadSettings(QSettings& s){
 // Fixed mode deliberately has one stable, documented range, including on upgrade.
 bottom=-100;top=-20;automaticScale=s.value("spectrum/mode","fixed").toString()=="automatic";
 contrast=setting(s,"spectrum/contrast",1,.25,4);brightness=setting(s,"spectrum/brightness",0,-1,1);
 smoothingFrames=int(setting(s,"spectrum/smoothingFrames",4,1,30));waterfallStride=int(setting(s,"spectrum/waterfallStride",1,1,30));displaySettingsChanged();
}
void Spectrum::saveSettings(QSettings& s)const{s.setValue("spectrum/mode",automaticScale?"automatic":"fixed");s.setValue("spectrum/minimum",-100);s.setValue("spectrum/maximum",-20);s.setValue("spectrum/contrast",contrast);s.setValue("spectrum/brightness",brightness);s.setValue("spectrum/smoothingFrames",smoothingFrames);s.setValue("spectrum/waterfallStride",waterfallStride);s.sync();}
void Spectrum::displaySettingsChanged(){recolor_=true;averagePower_.clear();update();}
void Spectrum::tuneFrequency(quint64 value){if(frequency!=value){levels_.clear();rawLevels_.clear();averagePower_.clear();waterfallDb_.clear();waterfall_=QImage();historyRows_=0;waterfallFrames=0;autoBottom_=-100;autoTop_=-20;}frequency=value;update();}
QVector<double> Spectrum::axisTicks()const{auto range=axisRange();QVector<double> ticks;for(int i=0;i<=8;++i)ticks<<range.second-i*(range.second-range.first)/8;return ticks;}
QRectF Spectrum::plotRect()const{QFontMetrics fm(font());double left=qMax(82,fm.horizontalAdvance(QStringLiteral("−100 dBFS"))+18);return QRectF(left,30,qMax(1.,width()-left-14),qMax(36.,height()*.62-60));}
QColor Spectrum::waterfallColor(double db)const{
 auto [low,high]=axisRange();double value=std::clamp(((db-low)/qMax(1.,high-low)-.5)*contrast+.5+brightness,0.,1.);
 // Navy -> royal blue -> azure -> ice cyan. No hue shift into green/red.
 const QColor palette[]{QColor("#020919"),QColor("#071c4c"),QColor("#123f91"),QColor("#1576ca"),QColor("#33b7ed"),QColor("#b9f1ff")};
 double at=value*5;int index=qMin(4,int(at));double mix=at-index;auto a=palette[index],b=palette[index+1];return QColor(qRound(a.red()+(b.red()-a.red())*mix),qRound(a.green()+(b.green()-a.green())*mix),qRound(a.blue()+(b.blue()-a.blue())*mix));
}
void Spectrum::rebuildWaterfall(){if(waterfall_.size()!=QSize(bins,rows)){waterfall_=QImage(bins,rows,QImage::Format_RGB32);waterfall_.fill(QColor("#020919"));}if(recolor_){for(int y=0;y<historyRows_;++y){auto line=reinterpret_cast<QRgb*>(waterfall_.scanLine(y));for(int x=0;x<bins;++x)line[x]=waterfallColor(waterfallDb_[y*bins+x]).rgb();}recolor_=false;}}
void Spectrum::updateIq(const QByteArray& data){
 if(data.size()<bins*2)return;std::array<std::complex<float>,bins> fft;
 for(int i=0;i<bins;++i)fft[i]=std::complex<float>((uchar(data[2*i])-127.5f)/128,(uchar(data[2*i+1])-127.5f)/128)*float(.5-.5*cos(2*std::numbers::pi*i/(bins-1)));
 for(int i=1,j=0;i<bins;++i){int bit=bins>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)std::swap(fft[i],fft[j]);}
 for(int len=2;len<=bins;len*=2){auto step=std::polar(1.f,float(-2*std::numbers::pi/len));for(int i=0;i<bins;i+=len){std::complex<float> phase=1;for(int j=0;j<len/2;++j){auto a=fft[i+j],b=fft[i+j+len/2]*phase;fft[i+j]=a+b;fft[i+j+len/2]=a-b;phase*=step;}}}
 rawLevels_.resize(bins);levels_.resize(bins);bool first=averagePower_.size()!=bins;if(first)averagePower_.resize(bins);double alpha=1./qBound(1,smoothingFrames,30);
 for(int i=0;i<bins;++i){double magnitude=std::max(1e-8f,std::abs(fft[(i+bins/2)%bins])/512);double power=magnitude*magnitude;rawLevels_[i]=float(20*log10(magnitude));averagePower_[i]=float(first?power:averagePower_[i]+alpha*(power-averagePower_[i]));levels_[i]=float(10*log10(std::max(1e-16f,averagePower_[i])));}
 if(automaticScale){auto sorted=rawLevels_;std::sort(sorted.begin(),sorted.end());double low=std::clamp(std::floor((double(sorted[bins/10])-10)/10)*10,-160.,-30.);double high=std::clamp(std::ceil((double(sorted.back())+10)/10)*10,low+20,0.);autoBottom_+=.15*(low-autoBottom_);autoTop_+=.15*(high-autoTop_);recolor_=true;}
 if(++waterfallFrames%qMax(1,waterfallStride)==0){if(waterfallDb_.size()!=bins*rows)waterfallDb_.fill(-160,bins*rows);std::memmove(waterfallDb_.data()+bins,waterfallDb_.constData(),sizeof(float)*bins*(rows-1));std::copy(rawLevels_.begin(),rawLevels_.end(),waterfallDb_.begin());historyRows_=qMin(rows,historyRows_+1);if(!waterfall_.isNull()&&!recolor_){for(int y=rows-1;y>0;--y)memcpy(waterfall_.scanLine(y),waterfall_.constScanLine(y-1),bins*4);auto line=reinterpret_cast<QRgb*>(waterfall_.scanLine(0));for(int x=0;x<bins;++x)line[x]=waterfallColor(rawLevels_[x]).rgb();}else recolor_=true;}
 update();
}
void Spectrum::paintEvent(QPaintEvent*){
 QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.fillRect(rect(),QColor("#020c1c"));auto plot=plotRect();auto [low,high]=axisRange();p.setPen(QColor("#b8d5eb"));p.drawText(QRectF(plot.left(),2,plot.width(),23),Qt::AlignLeft|Qt::AlignVCenter,automaticScale?"Échelle automatique (optionnelle)":"Échelle fixe : −20 à −100 dBFS");
 for(int i=0;i<=8;++i){double y=plot.top()+i*plot.height()/8;p.setPen(QPen(QColor("#19314c"),1));p.drawLine(QPointF(plot.left(),y),QPointF(plot.right(),y));p.setPen(QColor("#c2d7eb"));double tick=high-i*(high-low)/8;p.drawText(QRectF(0,y-10,plot.left()-9,20),Qt::AlignRight|Qt::AlignVCenter,QString::number(tick,'f',0).replace('-',QChar(0x2212))+" dBFS");}
 for(int i=0;i<=8;++i){double x=plot.left()+i*plot.width()/8;p.setPen(QColor("#19314c"));p.drawLine(QPointF(x,plot.top()),QPointF(x,plot.bottom()));}
 int visible=qBound(2,int(bins/qMax(1.,zoom)),bins),first=(bins-visible)/2;double span=sampleRate/qMax(1.,zoom);QRectF waterfall(plot.left(),plot.bottom()+30,plot.width(),qMax(1.,height()-plot.bottom()-48));
 p.save();p.setClipRect(plot);double band=bandwidth/span*plot.width();p.fillRect(QRectF(plot.center().x()-band/2,plot.top(),band,plot.height()),QColor(25,114,177,28));if(levels_.isEmpty()){p.setPen(QColor("#86a9c4"));p.drawText(plot,Qt::AlignCenter,"Aucune donnée IQ — spectre en attente");}else{QPainterPath curve;for(int i=0;i<visible;++i){double x=plot.left()+double(i)*plot.width()/(visible-1),y=plot.top()+std::clamp((high-levels_[first+i])/qMax(1.,high-low),0.,1.)*plot.height();if(!i)curve.moveTo(x,y);else curve.lineTo(x,y);}p.setPen(QPen(QColor(51,208,255,50),4));p.drawPath(curve);p.setPen(QPen(QColor("#55e3ff"),1.7));p.drawPath(curve);}p.restore();
 rebuildWaterfall();if(!waterfall_.isNull())p.drawImage(waterfall,waterfall_,QRectF(first,0,visible,rows));else p.fillRect(waterfall,QColor("#020919"));
 p.setPen(QColor("#99bad5"));for(int i=0;i<=4;++i){double f=frequency+(i/4.-.5)*span;auto text=QString::number(f/1e6,'f',6);double x=plot.left()+i*plot.width()/4;auto box=QRectF(x-60,plot.bottom()+4,120,22);if(i==0)box.moveLeft(plot.left());if(i==4)box.moveRight(plot.right());p.drawText(box,Qt::AlignCenter,text);}
 p.setRenderHint(QPainter::Antialiasing,false);p.setPen(QPen(QColor("#ff343e"),2));double center=plot.center().x();p.drawLine(QPointF(center,plot.top()),QPointF(center,plot.bottom()));p.drawLine(QPointF(center,waterfall.top()),QPointF(center,waterfall.bottom()));p.setPen(QColor("#91b5d0"));p.drawText(QRectF(plot.left(),height()-18,plot.width(),18),Qt::AlignRight|Qt::AlignVCenter,"Waterfall bleu • MHz • niveaux FFT bruts");
}

void Spectrum::mousePressEvent(QMouseEvent* event){auto plot=plotRect();if(event->button()==Qt::LeftButton&&event->position().x()>=plot.left()&&event->position().x()<=plot.right())emit tuneRequested(qRound64(frequency+((event->position().x()-plot.left())/plot.width()-.5)*sampleRate/qMax(1.,zoom)));}
