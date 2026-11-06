#include "fcdradio.h"
#include "iqconversion.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QElapsedTimer>
#include <QUuid>
#include <array>
#ifdef Q_OS_WIN
#include <windows.h>
#include <setupapi.h>
extern "C" {
#include <hidsdi.h>
#include <hidpi.h>
}
#include <mmsystem.h>
#include <mmddk.h>
namespace {
struct Handle{HANDLE value=INVALID_HANDLE_VALUE;~Handle(){if(value&&value!=INVALID_HANDLE_VALUE)CloseHandle(value);}};
void failure(QString text){throw std::runtime_error(text.toStdString());}
QString container(QString path){auto set=SetupDiCreateDeviceInfoList(nullptr,nullptr);if(set==INVALID_HANDLE_VALUE)return {};SP_DEVICE_INTERFACE_DATA iface{};iface.cbSize=sizeof(iface);QString result;if(SetupDiOpenDeviceInterfaceW(set,reinterpret_cast<const wchar_t*>(path.utf16()),0,&iface)){DWORD needed=0;SetupDiGetDeviceInterfaceDetailW(set,&iface,nullptr,0,&needed,nullptr);if(needed>0&&needed<65536){std::vector<BYTE> storage(needed,0);auto detail=reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(storage.data());detail->cbSize=sizeof(*detail);SP_DEVINFO_DATA device{};device.cbSize=sizeof(device);if(SetupDiGetDeviceInterfaceDetailW(set,&iface,detail,needed,nullptr,&device)){const DEVPROPKEY key={{0x8c7ed206,0x3f8a,0x4827,{0xb3,0xab,0xae,0x9e,0x1f,0xae,0xfc,0x6c}},2};GUID id{};DEVPROPTYPE type=0;if(SetupDiGetDevicePropertyW(set,&device,&key,&type,reinterpret_cast<BYTE*>(&id),sizeof(id),nullptr,0)&&type==DEVPROP_TYPE_GUID)result=QUuid(id.Data1,id.Data2,id.Data3,id.Data4[0],id.Data4[1],id.Data4[2],id.Data4[3],id.Data4[4],id.Data4[5],id.Data4[6],id.Data4[7]).toString();}}}SetupDiDestroyDeviceInfoList(set);return result;}
struct Input{UINT index;QString path,containerId,name;};
QVector<Input> inputs(){QVector<Input> out;for(UINT i=0;i<waveInGetNumDevs();++i){WAVEINCAPSW caps{};if(waveInGetDevCapsW(i,&caps,sizeof(caps))||caps.wChannels<2)continue;ULONG size=0;auto id=reinterpret_cast<HWAVEIN>(UINT_PTR(i));if(waveInMessage(id,DRV_QUERYDEVICEINTERFACESIZE,reinterpret_cast<DWORD_PTR>(&size),0)||!size||size>65536)continue;std::vector<wchar_t> path(size/2+1,0);if(waveInMessage(id,DRV_QUERYDEVICEINTERFACE,reinterpret_cast<DWORD_PTR>(path.data()),size))continue;QString value=QString::fromWCharArray(path.data());auto upper=value.toUpper();if(!upper.contains("VID_04D8")||(!upper.contains("PID_FB31")&&!upper.contains("PID_FB56")))continue;out<<Input{i,value,container(value),QString::fromWCharArray(caps.szPname)};}return out;}
QJsonObject args(QString key){if(!key.startsWith("fcd:"))failure("Identité FUNcube invalide");auto value=QJsonDocument::fromJson(QByteArray::fromBase64(key.mid(4).toLatin1(),QByteArray::Base64UrlEncoding)).object();if(value["hid"].toString().isEmpty()||value["audio"].toString().isEmpty()||value["container"].toString().isEmpty())failure("FUNcube : association HID/audio incomplète ; actualiser les périphériques");return value;}
Input resolve(QJsonObject value){auto audio=value["audio"].toString(),group=value["container"].toString();if(container(value["hid"].toString())!=group)failure("FUNcube : l’interface HID mémorisée n’appartient plus au même périphérique");for(auto input:inputs())if(input.path.compare(audio,Qt::CaseInsensitive)==0&&input.containerId==group)return input;failure("FUNcube : interface IQ audio absente ou changée ; aucune autre entrée audio ne sera utilisée");return {};}
WAVEFORMATEX format(UINT rate){WAVEFORMATEX out{};out.wFormatTag=WAVE_FORMAT_PCM;out.nChannels=2;out.nSamplesPerSec=rate;out.wBitsPerSample=16;out.nBlockAlign=4;out.nAvgBytesPerSec=rate*4;return out;}
QVector<double> rates(Input input){QVector<double> out;for(UINT rate:{48000u,96000u,192000u}){auto wave=format(rate);if(waveInOpen(nullptr,input.index,&wave,0,0,WAVE_FORMAT_QUERY|WAVE_FORMAT_DIRECT)==MMSYSERR_NOERROR)out<<rate;}if(out.isEmpty())failure("FUNcube : aucun format IQ PCM 16 bits stéréo direct disponible (48/96/192 kéch/s)");return out;}
class Hid {
 Handle handle;
public:USHORT inputLength=0,outputLength=0;
 explicit Hid(QString path){handle.value=CreateFileW(reinterpret_cast<const wchar_t*>(path.utf16()),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,nullptr);if(handle.value==INVALID_HANDLE_VALUE)failure("Ouverture HID FUNcube refusée : "+QString::number(GetLastError()));PHIDP_PREPARSED_DATA data=nullptr;if(!HidD_GetPreparsedData(handle.value,&data))failure("Descripteur HID FUNcube indisponible");HIDP_CAPS caps{};auto status=HidP_GetCaps(data,&caps);HidD_FreePreparsedData(data);if(status!=HIDP_STATUS_SUCCESS||caps.InputReportByteLength<7||caps.OutputReportByteLength<6||caps.InputReportByteLength>1024||caps.OutputReportByteLength>1024)failure("Rapports HID FUNcube incompatibles");inputLength=caps.InputReportByteLength;outputLength=caps.OutputReportByteLength;}
 DWORD io(bool write,QByteArray& bytes){Handle event;event.value=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!event.value)failure("Événement HID indisponible");OVERLAPPED overlapped{};overlapped.hEvent=event.value;DWORD count=0;BOOL done=write?WriteFile(handle.value,bytes.data(),bytes.size(),&count,&overlapped):ReadFile(handle.value,bytes.data(),bytes.size(),&count,&overlapped);if(!done){auto error=GetLastError();if(error!=ERROR_IO_PENDING)failure("Échange HID refusé : "+QString::number(error));if(WaitForSingleObject(event.value,750)!=WAIT_OBJECT_0){CancelIoEx(handle.value,&overlapped);GetOverlappedResult(handle.value,&overlapped,&count,TRUE);failure("Délai de réponse HID FUNcube dépassé");}if(!GetOverlappedResult(handle.value,&overlapped,&count,FALSE))failure("Lecture/écriture HID FUNcube interrompue");}return count;}
 QByteArray command(uchar command,quint32 value=0){HidD_FlushQueue(handle.value);QByteArray output(outputLength,0);output[1]=char(command);if(command==101)qToLittleEndian<quint32>(value,output.data()+2);if(io(true,output)!=DWORD(output.size()))failure("Écriture HID FUNcube partielle");QByteArray input(inputLength,0);auto count=io(false,input);input.resize(count);if(!input.isEmpty()&&input[0]==0)input.remove(0,1);if(input.size()<2||uchar(input[0])!=command||input[1]!=1)failure("FUNcube : réponse HID invalide ou commande refusée (mode firmware à vérifier)");return input;}
};
class Wave {
public:HWAVEIN handle=nullptr;Handle event;std::array<QByteArray,4> buffers;std::array<WAVEHDR,4> headers{};int prepared=0;
 ~Wave(){if(handle){waveInReset(handle);for(int i=0;i<prepared;++i)waveInUnprepareHeader(handle,&headers[i],sizeof(WAVEHDR));waveInClose(handle);}}
 void check(MMRESULT code){if(code){wchar_t text[256]{};waveInGetErrorTextW(code,text,256);failure("FUNcube IQ audio : "+QString::fromWCharArray(text)+" (code "+QString::number(code)+")");}}
 void open(Input input,quint32 rate){event.value=CreateEventW(nullptr,FALSE,FALSE,nullptr);if(!event.value)failure("Événement IQ audio indisponible");auto wave=format(rate);check(waveInOpen(&handle,input.index,&wave,reinterpret_cast<DWORD_PTR>(event.value),0,CALLBACK_EVENT|WAVE_FORMAT_DIRECT));for(int i=0;i<4;++i){buffers[i].resize(32768);headers[i].lpData=buffers[i].data();headers[i].dwBufferLength=buffers[i].size();check(waveInPrepareHeader(handle,&headers[i],sizeof(WAVEHDR)));++prepared;check(waveInAddBuffer(handle,&headers[i],sizeof(WAVEHDR)));}check(waveInStart(handle));}
};
class WindowsTransport:public FcdTransport {
 Input input;Hid hid;Wave wave;int next=0;
public:explicit WindowsTransport(QJsonObject value):input(resolve(value)),hid(value["hid"].toString()){}
 QVector<double> rates()override{return ::rates(input);}
 QByteArray command(uchar code,quint32 value=0)override{return hid.command(code,value);}
 void start(quint32 rate)override{wave.open(input,rate);}
 QByteArray readPcm(int timeoutMs)override{WaitForSingleObject(wave.event.value,timeoutMs);auto& header=wave.headers[next];if(!(header.dwFlags&WHDR_DONE))return {};if(header.dwBytesRecorded>header.dwBufferLength||header.dwBytesRecorded%4)failure("FUNcube : tampon IQ PCM16 tronqué");QByteArray result(header.lpData,int(header.dwBytesRecorded));header.dwBytesRecorded=0;wave.check(waveInAddBuffer(wave.handle,&header,sizeof(WAVEHDR)));next=(next+1)%4;return result;}
};
}
#endif
QVector<RadioDevice> FcdRadio::devices(QStringList* diagnostics){QVector<RadioDevice> out;
#ifdef Q_OS_WIN
 GUID guid{};HidD_GetHidGuid(&guid);auto set=SetupDiGetClassDevsW(&guid,nullptr,nullptr,DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);if(set==INVALID_HANDLE_VALUE)return out;auto audio=inputs();SP_DEVICE_INTERFACE_DATA iface{};iface.cbSize=sizeof(iface);for(DWORD i=0;SetupDiEnumDeviceInterfaces(set,nullptr,&guid,i,&iface);++i){DWORD size=0;SetupDiGetDeviceInterfaceDetailW(set,&iface,nullptr,0,&size,nullptr);if(!size||size>65536)continue;std::vector<BYTE> storage(size,0);auto detail=reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(storage.data());detail->cbSize=sizeof(*detail);if(!SetupDiGetDeviceInterfaceDetailW(set,&iface,detail,size,nullptr,nullptr))continue;QString path=QString::fromWCharArray(detail->DevicePath);auto upper=path.toUpper();bool plus=upper.contains("PID_FB31");if(!upper.contains("VID_04D8")||(!plus&&!upper.contains("PID_FB56")))continue;auto group=container(path);QVector<Input> matches;for(auto input:audio)if(!group.isEmpty()&&input.containerId==group)matches<<input;QJsonObject value{{"hid",path},{"audio",matches.size()==1?matches[0].path:QString{}},{"container",group},{"plus",plus}};RadioDevice device;device.name=plus?"FUNcube Dongle Pro+":"FUNcube Dongle Pro";device.key="fcd:"+QString::fromLatin1(QJsonDocument(value).toJson(QJsonDocument::Compact).toBase64(QByteArray::Base64UrlEncoding));device.usbId=plus?"04D8:FB31":"04D8:FB56";device.driver="Windows HID + USB Audio / PCM16 stéréo";device.usable=matches.size()==1;device.status=device.usable?"Interfaces HID et IQ audio associées par ContainerId — réception à tester":"Interface HID visible ; association audio unique absente";device.resolution="Conserver HID/USB Audio Windows, vérifier l’interface audio du même appareil ; ne pas remplacer par WinUSB.";out<<device;}SetupDiDestroyDeviceInfoList(set);if(diagnostics)*diagnostics<<QString("FUNcube Windows : %1 contrôle(s) HID, association audio stricte ; aucune entrée microphone par défaut utilisée").arg(out.size());
#else
 if(diagnostics)*diagnostics<<"FUNcube natif : Windows requis";
#endif
 return out;}
SdrCapabilities FcdRadio::capabilities(QString key){SdrCapabilities result;
#ifdef Q_OS_WIN
 try{auto value=args(key);auto input=resolve(value);Hid hid(value["hid"].toString());auto query=hid.command(1);if(!query.mid(2).startsWith("FCDAPP"))failure("FUNcube en mode bootloader ou firmware incompatible");result.backend="FUNcube natif Windows — HID + PCM16 stéréo direct, PPM logiciel ; gain RF conservé";result.rates=rates(input);result.rateMin=result.rates.first();result.rateMax=result.rates.last();result.ppm=true;}catch(const std::exception& e){result.error=QString::fromUtf8(e.what());}
#else
 result.error="FUNcube natif : Windows requis";
#endif
 return result;}
void FcdRadio::stream(const RadioConfig& config,std::atomic_bool& stopping,const std::function<void(QByteArray)>& samples,const std::function<void(quint32,quint32)>& tuned,FcdTransport& transport){
 auto fail=[](QString text){throw std::runtime_error(text.toStdString());};
 if(!transport.rates().contains(config.rate))fail("FUNcube : taux PCM16 stéréo non annoncé");
 if(config.automaticGain||config.bandwidth||!config.antenna.isEmpty())fail("AGC, bande ou antenne non configurables par cet adaptateur FUNcube");
 auto command=[&](uchar code,quint32 value=0){auto response=transport.command(code,value);if(response.size()<2||uchar(response[0])!=code||response[1]!=1)fail("FUNcube : commande HID refusée ou réponse invalide");return response;};
 if(!command(1).mid(2).startsWith("FCDAPP"))fail("FUNcube hors mode application");
 double corrected=config.frequency*(1+config.ppm/1e6);if(corrected<1||corrected>4294967295.)fail("Fréquence FUNcube corrigée PPM hors plage");auto target=quint32(std::llround(corrected));command(101,target);auto read=command(102);if(read.size()<6||qFromLittleEndian<quint32>(read.constData()+2)!=target)fail("FUNcube : fréquence relue différente de la demande");
 transport.start(config.rate);tuned(config.frequency,config.rate);QElapsedTimer last;last.start();while(!stopping){auto bytes=transport.readPcm(50);if(!bytes.isEmpty()){if(bytes.size()%4)fail("FUNcube : tampon IQ PCM16 tronqué");samples(IqConversion::pcm16(reinterpret_cast<const unsigned char*>(bytes.constData()),bytes.size()/4));last.restart();}if(last.elapsed()>3000)fail("FUNcube : absence d’IQ depuis trois secondes ; vérifier câble, pilote audio et application concurrente");}
}
void FcdRadio::receive(const RadioConfig& config,std::atomic_bool& stopping,const std::function<void(QByteArray)>& samples,const std::function<void(quint32,quint32)>& tuned){
#ifdef Q_OS_WIN
 WindowsTransport transport(args(config.deviceKey));stream(config,stopping,samples,tuned,transport);
#else
 throw std::runtime_error("FUNcube natif : Windows requis");
#endif
}
