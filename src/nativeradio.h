#pragma once
#include "soapyradio.h"
namespace NativeRadio {
bool supports(QString key);
QVector<RadioDevice> devices(QStringList* diagnostics=nullptr);
SdrCapabilities capabilities(QString key);
void receive(const RadioConfig& config,std::atomic_bool& stopping,
 const std::function<void(QByteArray)>& samples,const std::function<void(quint32,quint32)>& tuned,
 const std::function<void(QString)>& diagnostic);
}
