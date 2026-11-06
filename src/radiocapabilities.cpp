#include "radiocatalog.h"
#include "nativeradio.h"
#include <QLibrary>
#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>
#include <QScopeGuard>
#ifdef Q_OS_WIN
#include <windows.h>
#include <winsvc.h>
#endif
namespace {
template<class T>T fn(QLibrary& lib,const char* name){auto value=lib.resolve(name);if(!value)throw std::runtime_error(("API absente : "+QString::fromLatin1(name)).toStdString());return reinterpret_cast<T>(value);}
}
SdrCapabilities RadioCatalog::capabilities(QString key){if(NativeRadio::supports(key))return NativeRadio::capabilities(key);if(key.startsWith("soapy:"))return SoapyRadio::capabilities(key);SdrCapabilities c;try{
 if(key.startsWith("hackrf:")){c.backend="HackRF natif — limites de l’API, paramètres vérifiés à l’ouverture ; PPM logiciel";c.rates={2000000,4000000,8000000,10000000,20000000};c.rateMin=2000000;c.rateMax=20000000;c.gains={"LNA (pas 8 dB)","VGA (pas 2 dB)"};c.gainMax=40;c.bandwidth=true;c.ppm=true;return c;}
 if(!key.startsWith("rtl:"))throw std::runtime_error("Aucune API RX associée à cette ligne de diagnostic");QLibrary lib(qEnvironmentVariable("POCSAG_TEST_RTL_LIBRARY","rtlsdr"));if(!lib.load())throw std::runtime_error(lib.errorString().toStdString());auto count=fn<uint32_t(*)()>(lib,"rtlsdr_get_device_count");auto strings=fn<int(*)(uint32_t,char*,char*,char*)>(lib,"rtlsdr_get_device_usb_strings");int index=-1,matches=0;for(uint32_t i=0;i<count();++i){char m[256]{},p[256]{},s[256]{};if(strings(i,m,p,s)==0&&key=="rtl:"+QString::fromUtf8(s)){index=i;++matches;}}if(matches!=1)throw std::runtime_error("RTL absent ou série partagée : identification unique requise");void* device=nullptr;auto close=fn<int(*)(void*)>(lib,"rtlsdr_close");auto guard=qScopeGuard([&]{if(device)close(device);});if(fn<int(*)(void**,uint32_t)>(lib,"rtlsdr_open")(&device,index)<0||!device)throw std::runtime_error("Ouverture RTL refusée : pilote WinUSB, accès USB ou application concurrente à vérifier");auto gains=fn<int(*)(void*,int*)>(lib,"rtlsdr_get_tuner_gains");int n=gains(device,nullptr);if(n<0||n>1024)throw std::runtime_error("Nombre de gains RTL invalide");if(n){std::vector<int> values(n);if(gains(device,values.data())!=n)throw std::runtime_error("Lecture des gains RTL incomplète");c.gainMin=*std::min_element(values.begin(),values.end())/10.;c.gainMax=*std::max_element(values.begin(),values.end())/10.;c.gains={"Gains tuner interrogés"};}c.backend="RTL-SDR natif — gains interrogés, taux proposés et accord vérifiés à l’ouverture";c.rates={240000,256000,1024000,2048000,2400000};c.rateMin=240000;c.rateMax=2400000;c.agc=true;c.ppm=true;c.bandwidth=lib.resolve("rtlsdr_set_tuner_bandwidth")!=nullptr;
 }catch(const std::exception& e){c.error=QString::fromUtf8(e.what());}return c;}
QString RadioCatalog::dependencies(){QStringList lines;for(auto name:{"rtlsdr.dll","libhackrf.dll","libairspy.dll","libairspyhf.dll","libSoapySDR.dll"}){auto path=QCoreApplication::applicationDirPath()+"/"+name;QLibrary lib(path);bool loaded=lib.load();lines<<QString("%1 : %2%3").arg(name,loaded?"chargeable x64":QFileInfo::exists(path)?"présente mais non chargeable (dépendance/architecture à vérifier)":"absente du dossier du programme",loaded?QString{}:" — "+lib.errorString());}
 QSettings sdk("HKEY_LOCAL_MACHINE\\SOFTWARE\\SDRplay\\Service\\API",QSettings::NativeFormat);auto root=sdk.value("Install_Dir").toString();lines<<"SDRplay API constructeur : "+(root.isEmpty()?QString("aucun emplacement enregistré détecté ; installation constructeur requise"):root);lines<<"SDRplay : module SoapySDRPlay3 et API constructeur compatibles requis. Ne pas remplacer le pilote constructeur par WinUSB.";
#ifdef Q_OS_WIN
 auto manager=OpenSCManagerW(nullptr,nullptr,SC_MANAGER_CONNECT);if(manager){auto service=OpenServiceW(manager,L"SDRplayAPIService",SERVICE_QUERY_STATUS);if(service){SERVICE_STATUS status{};if(QueryServiceStatus(service,&status))lines<<QString("SDRplayAPIService : %1").arg(status.dwCurrentState==SERVICE_RUNNING?"en cours":"arrêté ou en transition");else lines<<"État du service SDRplay non lisible";CloseServiceHandle(service);}else lines<<"SDRplayAPIService : absent ou non accessible";CloseServiceHandle(manager);}else lines<<"Gestionnaire de services non accessible en lecture";
#endif
 lines<<SoapyRadio::diagnostics();lines<<"FUNcube : pilotes HID et USB Audio Windows, association par ContainerId ; aucune DLL propriétaire ni entrée audio par défaut.";return lines.join('\n');}
void RadioCatalog::configureRuntime(QString path){QSettings settings(path,QSettings::IniFormat);auto library=settings.value("sdr/soapyLibrary").toString(),modules=settings.value("sdr/soapyModules").toString();if(qEnvironmentVariableIsEmpty("POCSAG_SOAPY_LIBRARY")&&!library.isEmpty())qputenv("POCSAG_SOAPY_LIBRARY",library.toUtf8());if(qEnvironmentVariableIsEmpty("SOAPY_SDR_PLUGIN_PATH")&&!modules.isEmpty())qputenv("SOAPY_SDR_PLUGIN_PATH",modules.toUtf8());}
