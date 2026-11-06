#include "nativeradio.h"
#include "radiocatalog.h"
#include "fcdradio.h"
#include "iqconversion.h"
#include "dsp.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QLibrary>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <thread>
#include <chrono>
static int failures=0;
void require(bool value,const char* name){if(!value){std::cerr<<"FAIL "<<name<<'\n';++failures;}}
template<class F>bool throws(F f){try{f();return false;}catch(const std::exception&){return true;}}
QString soapy(QString driver,QString serial){return "soapy:"+QJsonDocument(QJsonObject{{"driver",driver},{"serial",serial}}).toJson(QJsonDocument::Compact).toBase64(QByteArray::Base64UrlEncoding);}
struct FcdMock: FcdTransport {
 QByteArray pcm;std::atomic_bool* stop;quint32 frequency=0;int fault=0;
 QVector<double> rates()override{return {240000};}
 QByteArray command(uchar code,quint32 value)override{QByteArray out(6,0);out[0]=char(code);out[1]=fault==1?0:1;if(code==1)out=QByteArray("\1\1FCDAPP",8);if(code==101)frequency=value;if(code==102)qToLittleEndian<quint32>(frequency+(fault==2?1:0),out.data()+2);return out;}
 void start(quint32 rate)override{if(rate!=240000)throw std::runtime_error("bad rate");}
 QByteArray readPcm(int)override{*stop=true;return fault==3?QByteArray(3,0):pcm;}
};
int main(int argc,char** argv){QCoreApplication app(argc,argv);QTemporaryDir temp;require(temp.isValid(),"temporary data");QFile zipped(QString::fromUtf8(FIXTURE_DIRECTORY)+"/1200-alpha.iqz");require(zipped.open(QIODevice::ReadOnly),"independent reference opens");auto iq=qUncompress(zipped.readAll());require(!iq.isEmpty(),"reference data");QFile file(temp.filePath("reference.cu8"));require(file.open(QIODevice::WriteOnly),"fixture copy");file.write(iq);file.close();QString path=QCoreApplication::applicationDirPath()+"/mock_airspy.dll";QLibrary dll(path);require(dll.load(),"mock ABI DLL loads");auto set=reinterpret_cast<void(*)(const char*)>(dll.resolve("mock_fixture"));auto fault=reinterpret_cast<void(*)(int)>(dll.resolve("mock_fault"));auto closed=reinterpret_cast<int(*)()>(dll.resolve("mock_closed"));if(!set||!fault||!closed)return 2;set(file.fileName().toUtf8().constData());qputenv("POCSAG_TEST_AIRSPY_LIBRARY",path.toUtf8());qputenv("POCSAG_TEST_AIRSPYHF_LIBRARY",path.toUtf8());
 float limits[]{-2,-1,0,1,2,.5};auto bytes=IqConversion::floats(limits,3);require(uchar(bytes[0])==0&&uchar(bytes[1])==0&&uchar(bytes[2])==128&&uchar(bytes[3])==255&&uchar(bytes[4])==255,"CF32 saturation / zero");float bad[]{NAN,0};require(throws([&]{IqConversion::floats(bad,1);}),"nonfinite rejected");require(throws([&]{IqConversion::floats(limits,-1);}),"negative count rejected");
 std::vector<float> allValues;QByteArray allBytes(256,0),allPcm(512,0);for(int i=0;i<256;++i){allValues.push_back((i-127.5f)/128.f);allBytes[i]=char(i);qToLittleEndian<qint16>(qint16((i-128)*256),allPcm.data()+i*2);}require(IqConversion::floats(allValues.data(),128)==allBytes,"all 256 CF32 values round-trip");require(IqConversion::pcm16(reinterpret_cast<const unsigned char*>(allPcm.constData()),128)==allBytes,"all 256 PCM16 values round-trip");
 require(RadioCatalog::samePhysical("airspy:00001234",soapy("airspy","0x1234")),"busy identity across backend representations");auto merged=RadioCatalog::merge({{"native","airspy:0000000000001234"},{"alias",soapy("airspy","0x1234")},{"other","airspy:5678"}});require(merged.size()==2&&merged[0].serial=="0000000000001234","physical dedup preserves actual serial");auto ambiguous=RadioCatalog::merge({{"one","rtl:00000001"},{"two","rtl:00000001"}});require(ambiguous.size()==2&&!ambiguous[0].usable&&!ambiguous[1].usable,"shared serial disabled");require(RadioCatalog::serial("rtl-index:0").isEmpty(),"no index identity");
 QStringList diagnostics;auto found=NativeRadio::devices(&diagnostics);int airspies=0;for(auto d:found)if(d.key.startsWith("airspy"))++airspies;require(airspies==4,"two native families enumerate two serials");
 for(auto key:{QString("airspy:1234"),QString("airspyhf:5678")}){auto caps=NativeRadio::capabilities(key);require(caps.error.isEmpty()&&caps.rates.contains(240000),"capabilities queried from SDK");std::atomic_bool stop=false;RadioConfig config;config.deviceKey=key;config.frequency=169626000;config.rate=240000;config.gain=100;int frames=0,tuned=0; pocsag::Demodulator demod([&](pocsag::Message m){if(m.text=="REFERENCE 012 FUNCTION TEST"){++frames;stop=true;}});std::thread deadline([&]{for(int i=0;i<200&&!stop;++i)std::this_thread::sleep_for(std::chrono::milliseconds(10));stop=true;});NativeRadio::receive(config,stop,[&](QByteArray b){demod.cu8(std::span(reinterpret_cast<const unsigned char*>(b.constData()),size_t(b.size())));},[&](quint32 f,quint32 r){if(f==169626000&&r==240000)++tuned;},[](QString){});stop=true;deadline.join();require(frames>0&&tuned==1,"CF32 independent signal decodes through native ABI");
 for(int mode:{1,3,4}){stop=false;fault(mode);require(throws([&]{NativeRadio::receive(config,stop,[](QByteArray){},[](quint32,quint32){},[](QString){});}),"unplug / invalid float / refused tune fail visibly");fault(0);}config.rate=123456;stop=false;require(throws([&]{NativeRadio::receive(config,stop,[](QByteArray){},[](quint32,quint32){},[](QString){});}),"unsupported rate rejected");}
 std::atomic_bool stop=false;FcdMock fcd;fcd.stop=&stop;fcd.pcm.resize(iq.size()*2);for(int i=0;i<iq.size();++i)qToLittleEndian<qint16>(qint16((int(uchar(iq[i]))-128)*256),fcd.pcm.data()+i*2);RadioConfig config;config.frequency=169626000;config.rate=240000;int frames=0;pocsag::Demodulator demod([&](pocsag::Message m){if(m.text=="REFERENCE 012 FUNCTION TEST")++frames;});FcdRadio::stream(config,stop,[&](QByteArray b){demod.cu8(std::span(reinterpret_cast<const unsigned char*>(b.constData()),size_t(b.size())));},[](quint32,quint32){},fcd);demod.finish();require(frames>0,"PCM16 HID protocol reference decodes");for(int mode:{1,2,3}){fcd.fault=mode;stop=false;require(throws([&]{FcdRadio::stream(config,stop,[](QByteArray){},[](quint32,quint32){},fcd);}),"HID ack / readback / partial PCM rejected");}require(closed()>=10,"SDK sessions closed on success and failures");qputenv("POCSAG_TEST_AIRSPY_LIBRARY",temp.filePath("absent.dll").toUtf8());require(!NativeRadio::capabilities("airspy:1234").error.isEmpty(),"missing DLL explicit error");std::cout<<"Native SDR mock tests failures="<<failures<<" (not hardware qualification)\n";return failures?1:0;}
