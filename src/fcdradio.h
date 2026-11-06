#pragma once
#include "soapyradio.h"
// Hardware-independent HID/PCM session contract; tests inject an isolated transport.
class FcdTransport {
public:virtual ~FcdTransport()=default;
 virtual QVector<double> rates()=0;
 virtual QByteArray command(uchar code,quint32 value=0)=0;
 virtual void start(quint32 rate)=0;
 virtual QByteArray readPcm(int timeoutMs)=0;
};
namespace FcdRadio {
QVector<RadioDevice> devices(QStringList* diagnostics=nullptr);
SdrCapabilities capabilities(QString key);
void receive(const RadioConfig&,std::atomic_bool&,const std::function<void(QByteArray)>&,const std::function<void(quint32,quint32)>&);
void stream(const RadioConfig&,std::atomic_bool&,const std::function<void(QByteArray)>&,const std::function<void(quint32,quint32)>&,FcdTransport&);
}
