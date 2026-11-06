#include "radiocatalog.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QSet>
namespace {
QJsonObject args(QString key){return QJsonDocument::fromJson(QByteArray::fromBase64(key.mid(6).toLatin1(),QByteArray::Base64UrlEncoding)).object();}
QString normalized(QString family,QString serial){if(family=="airspy"||family=="airspyhf"||family=="hackrf"){serial=serial.toLower();if(serial.startsWith("0x"))serial.remove(0,2);while(serial.size()>1&&serial.startsWith('0'))serial.remove(0,1);}return serial;}
}
QString RadioCatalog::family(QString key){if(key.startsWith("rtl-index:"))return "rtl";if(!key.startsWith("soapy:"))return key.section(':',0,0);auto driver=args(key)["driver"].toString().toLower();if(driver=="rtlsdr")return "rtl";return driver;}
QString RadioCatalog::serial(QString key){if(key.startsWith("rtl-index:"))return {};if(key.startsWith("fcd:"))return QJsonDocument::fromJson(QByteArray::fromBase64(key.mid(4).toLatin1(),QByteArray::Base64UrlEncoding)).object()["serial"].toString();return key.startsWith("soapy:")?args(key)["serial"].toString():key.section(':',1);}
bool RadioCatalog::samePhysical(QString a,QString b){if(a==b)return true;auto f=family(a),s=normalized(f,serial(a));return !s.isEmpty()&&f==family(b)&&s==normalized(f,serial(b));}
QVector<RadioDevice> RadioCatalog::merge(QVector<RadioDevice> devices){
 QMap<QString,int> nativeCounts;for(auto& d:devices){auto f=family(d.key),s=normalized(f,serial(d.key));if(!d.key.startsWith("soapy:")&&!s.isEmpty())++nativeCounts[f+":"+s];}
 QVector<RadioDevice> out;QSet<QString> keys;for(auto d:devices){auto f=family(d.key),s=normalized(f,serial(d.key));d.backend=d.key.startsWith("soapy:")?"SoapySDR / "+f:f+" natif";d.serial=serial(d.key);if(d.key.startsWith("soapy:")&&!s.isEmpty()&&nativeCounts.value(f+":"+s)>0)continue;
 if(!d.key.startsWith("soapy:")&&!s.isEmpty()&&nativeCounts.value(f+":"+s)>1){d.usable=false;d.status="Identité ambiguë : numéro de série partagé";d.resolution="Attribuer des numéros de série uniques avec l’outil du constructeur avant réception. Aucun choix arbitraire par index.";d.key+="?ambiguous="+QString::number(out.size());}
 if(keys.contains(d.key))continue;keys.insert(d.key);if(d.status.isEmpty())d.status="Détecté par l’API — ouverture IQ à tester";out<<d;}return out;
}
