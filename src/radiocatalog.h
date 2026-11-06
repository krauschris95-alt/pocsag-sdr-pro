#pragma once
#include "receiver.h"
#include "soapyradio.h"
namespace RadioCatalog {
QVector<RadioDevice> merge(QVector<RadioDevice> devices);
QVector<RadioDevice> windowsUsb();
QString family(QString key);
QString serial(QString key);
bool samePhysical(QString first,QString second);
SdrCapabilities capabilities(QString key);
QString dependencies();
void configureRuntime(QString settingsFile);
}
