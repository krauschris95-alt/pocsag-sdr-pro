#pragma once
#include <QByteArray>
#include <QtEndian>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
namespace IqConversion {
inline QByteArray floats(const float* data,int count){
 if(!data||count<0||count>4*1024*1024)throw std::runtime_error("Tampon IQ CF32 invalide");
 QByteArray out(count*2,0);for(int i=0;i<count*2;++i){double value=data[i];if(!std::isfinite(value))throw std::runtime_error("Échantillon IQ CF32 non fini");out[i]=char(std::clamp(int(std::lround(std::clamp(value,-1.,1.)*128+127.5)),0,255));}return out;
}
inline QByteArray pcm16(const unsigned char* data,int count){
 if(!data||count<0||count>4*1024*1024)throw std::runtime_error("Tampon IQ CS16 invalide");
 QByteArray out(count*2,0);for(int i=0;i<count*2;++i){auto value=qFromLittleEndian<qint16>(data+i*2);out[i]=char((int(value)+32768)>>8);}return out;
}
}
