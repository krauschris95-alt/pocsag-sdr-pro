#pragma once
#include "receiver.h"
#include <functional>
struct SdrCapabilities {
 QString backend,error;QStringList antennas,gains;QVector<double> rates;
 double frequencyMin=0,frequencyMax=0,gainMin=0,gainMax=0,rateMin=0,rateMax=0;
 bool agc=false,ppm=false,bandwidth=false;
};
// An adapter owns its library, device and stream on the receiver thread.
// No decoder or GUI dependency: further adapters use the same CU8 consumer.
namespace SoapyRadio {
 QVector<RadioDevice> devices(QString* diagnostic=nullptr);
 QString diagnostics();
 SdrCapabilities capabilities(QString key);
 void receive(const RadioConfig& config,std::atomic_bool& stopping,
              const std::function<void(QByteArray)>& samples,const std::function<void(quint32,quint32)>& tuned,
              const std::function<void(QString)>& problem);
}
