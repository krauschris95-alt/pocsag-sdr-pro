#include "radiocatalog.h"
#include <QRegularExpression>
#include <vector>
#ifdef Q_OS_WIN
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
namespace {
QString property(HDEVINFO set,SP_DEVINFO_DATA& dev,DWORD key){DWORD type=0,needed=0;SetupDiGetDeviceRegistryPropertyW(set,&dev,key,&type,nullptr,0,&needed);if(!needed||needed>65536)return {};std::vector<BYTE> data(needed+sizeof(wchar_t),0);if(!SetupDiGetDeviceRegistryPropertyW(set,&dev,key,&type,data.data(),needed,nullptr))return {};return QString::fromWCharArray(reinterpret_cast<wchar_t*>(data.data()));}
}
#endif
QVector<RadioDevice> RadioCatalog::windowsUsb(){QVector<RadioDevice> out;
#ifdef Q_OS_WIN
 auto set=SetupDiGetClassDevsW(nullptr,nullptr,nullptr,DIGCF_ALLCLASSES|DIGCF_PRESENT);if(set==INVALID_HANDLE_VALUE)return out;SP_DEVINFO_DATA dev{};dev.cbSize=sizeof(dev);for(DWORD i=0;SetupDiEnumDeviceInfo(set,i,&dev);++i){auto hardware=property(set,dev,SPDRP_HARDWAREID).toUpper();if(!hardware.startsWith("USB\\VID_"))continue;QString f;auto id=QRegularExpression("VID_([0-9A-F]{4})&PID_([0-9A-F]{4})").match(hardware);if(!id.hasMatch())continue;auto usb=id.captured(1)+":"+id.captured(2);if(usb=="0BDA:2838"||usb=="0BDA:2832")f="RTL-SDR";else if(usb=="1D50:6089")f="HackRF";else if(usb=="1D50:60A1")f="Airspy";else if(usb=="03EB:800C")f="Airspy HF+";else if(usb=="04D8:FB31")f="FUNcube Dongle Pro+";else if(usb=="04D8:FB56")f="FUNcube Dongle Pro";else if(id.captured(1)=="1DF7")f="SDRplay (modèle fourni par Windows/API)";else if(usb=="1D50:6108")f="LimeSDR";else if(id.captured(1)=="2CF0")f="bladeRF (modèle fourni par Windows/API)";else if(usb=="0456:B673")f="PlutoSDR";if(f.isEmpty())continue;
 wchar_t instance[4096]{};if(!SetupDiGetDeviceInstanceIdW(set,&dev,instance,4096,nullptr))continue;RadioDevice d;d.key="usb:"+QString::fromWCharArray(instance);auto name=property(set,dev,SPDRP_FRIENDLYNAME);if(name.isEmpty())name=property(set,dev,SPDRP_DEVICEDESC);d.name=f+" — "+name;d.backend="Inventaire Windows uniquement";d.usbId=hardware;d.driver=property(set,dev,SPDRP_SERVICE);if(d.driver.isEmpty())d.driver="Aucun service de pilote annoncé";ULONG status=0,problem=0;if(CM_Get_DevNode_Status(&status,&problem,dev.DevInst,0)==CR_SUCCESS)d.status=QString("Windows : état 0x%1, code problème %2 ; IQ non vérifié").arg(status,0,16).arg(problem);else d.status="État Windows indisponible ; IQ non vérifié";d.usable=false;d.resolution=(f.startsWith("FUNcube")?"Conserver les pilotes audio USB et HID Windows. Ne pas les remplacer par WinUSB. ":"Vérifier la documentation du constructeur avant toute modification de pilote. ")+QString("Installer le SDK/module x64 adapté si aucune ligne API correspondante n’est disponible.");out<<d;}SetupDiDestroyDeviceInfoList(set);
#endif
 return out;}
