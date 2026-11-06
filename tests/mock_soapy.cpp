// C API simulator only. It cannot validate a vendor driver or a physical SDR.
#define SOAPY_SDR_DLL_EXPORTS
#include <SoapySDR/Device.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fstream>
#include <algorithm>
#include <limits>
static std::vector<float> fixture;static int fault=0;
struct State{double frequency=0,rate=0;size_t pos=0;};
extern "C" __declspec(dllexport) void mock_fixture(const char* path){std::ifstream input(path,std::ios::binary);std::vector<unsigned char> data(std::istreambuf_iterator<char>(input),{});fixture.clear();for(auto b:data)fixture.push_back((int(b)-127.5f)/128.f);}
extern "C" __declspec(dllexport) void mock_fault(int value){fault=value;}
extern "C" __declspec(dllexport) const char* SoapySDR_getABIVersion(){return fault==6?"9.9":"0.8";}
void SoapySDR_free(void* p){free(p);}
int SoapySDRKwargs_set(SoapySDRKwargs* a,const char* key,const char* value){a->keys=static_cast<char**>(realloc(a->keys,(a->size+1)*sizeof(char*)));a->vals=static_cast<char**>(realloc(a->vals,(a->size+1)*sizeof(char*)));a->keys[a->size]=strdup(key);a->vals[a->size]=strdup(value);++a->size;return 0;}
void SoapySDRKwargs_clear(SoapySDRKwargs* a){for(size_t i=0;i<a->size;++i){free(a->keys[i]);free(a->vals[i]);}free(a->keys);free(a->vals);*a={};}
void SoapySDRKwargsList_clear(SoapySDRKwargs* a,size_t n){for(size_t i=0;i<n;++i)SoapySDRKwargs_clear(a+i);free(a);}
void SoapySDRStrings_clear(char*** p,size_t n){for(size_t i=0;i<n;++i)free((*p)[i]);free(*p);*p=nullptr;}
SoapySDRKwargs* SoapySDRDevice_enumerate(const SoapySDRKwargs*,size_t* n){*n=4;auto a=static_cast<SoapySDRKwargs*>(calloc(*n,sizeof(SoapySDRKwargs)));const char* drivers[]{"sdrplay","lime","bladerf","plutosdr"};for(size_t i=0;i<*n;++i){SoapySDRKwargs_set(a+i,"driver",drivers[i]);SoapySDRKwargs_set(a+i,"serial","SIMULATED-ONLY");SoapySDRKwargs_set(a+i,"label",drivers[i]);}return a;}
const char* SoapySDRDevice_lastError(){return "SIMULATED vendor failure — not physical hardware";}
SoapySDRDevice* SoapySDRDevice_make(const SoapySDRKwargs*){return fault==5?nullptr:reinterpret_cast<SoapySDRDevice*>(new State);}
int SoapySDRDevice_unmake(SoapySDRDevice* p){delete reinterpret_cast<State*>(p);return 0;}
size_t SoapySDRDevice_getNumChannels(const SoapySDRDevice*,int){return 1;}
SoapySDRRange* SoapySDRDevice_getFrequencyRange(const SoapySDRDevice*,int,size_t,size_t* n){*n=1;auto p=static_cast<SoapySDRRange*>(calloc(1,sizeof(SoapySDRRange)));*p={100000,2000000000,1};return p;}
SoapySDRRange SoapySDRDevice_getGainRange(const SoapySDRDevice*,int,size_t){return {0,50,1};}
bool SoapySDRDevice_hasGainMode(const SoapySDRDevice*,int,size_t){return true;}
bool SoapySDRDevice_hasFrequencyCorrection(const SoapySDRDevice*,int,size_t){return true;}
double* SoapySDRDevice_listSampleRates(const SoapySDRDevice*,int,size_t,size_t* n){*n=1;auto p=static_cast<double*>(malloc(sizeof(double)));*p=240000;return p;}
SoapySDRRange* SoapySDRDevice_getSampleRateRange(const SoapySDRDevice*,int,size_t,size_t* n){*n=1;auto p=static_cast<SoapySDRRange*>(calloc(1,sizeof(SoapySDRRange)));*p={240000,240000,0};return p;}
SoapySDRRange* SoapySDRDevice_getBandwidthRange(const SoapySDRDevice*,int,size_t,size_t* n){*n=0;return nullptr;}
char** SoapySDRDevice_listAntennas(const SoapySDRDevice*,int,size_t,size_t* n){*n=0;return nullptr;}
char** SoapySDRDevice_listGains(const SoapySDRDevice*,int,size_t,size_t* n){*n=0;return nullptr;}
int SoapySDRDevice_setSampleRate(SoapySDRDevice* p,int,size_t,double rate){reinterpret_cast<State*>(p)->rate=rate;return 0;}
int SoapySDRDevice_setFrequency(SoapySDRDevice* p,int,size_t,double f,const SoapySDRKwargs*){reinterpret_cast<State*>(p)->frequency=f;return 0;}
int SoapySDRDevice_setFrequencyCorrection(SoapySDRDevice*,int,size_t,double){return 0;}
int SoapySDRDevice_setGainMode(SoapySDRDevice*,int,size_t,bool){return 0;}
int SoapySDRDevice_setGain(SoapySDRDevice*,int,size_t,double){return 0;}
int SoapySDRDevice_setBandwidth(SoapySDRDevice*,int,size_t,double){return 0;}
int SoapySDRDevice_setAntenna(SoapySDRDevice*,int,size_t,const char*){return 0;}
double SoapySDRDevice_getSampleRate(const SoapySDRDevice* p,int,size_t){return reinterpret_cast<const State*>(p)->rate+(fault==1?1000:0);}
double SoapySDRDevice_getFrequency(const SoapySDRDevice* p,int,size_t){return reinterpret_cast<const State*>(p)->frequency;}
SoapySDRStream* SoapySDRDevice_setupStream(SoapySDRDevice* p,int,const char* format,const size_t*,size_t,const SoapySDRKwargs*){return strcmp(format,"CF32")?nullptr:reinterpret_cast<SoapySDRStream*>(p);}
int SoapySDRDevice_activateStream(SoapySDRDevice*,SoapySDRStream*,int,long long,size_t){return 0;}
int SoapySDRDevice_deactivateStream(SoapySDRDevice*,SoapySDRStream*,int,long long){return 0;}
int SoapySDRDevice_closeStream(SoapySDRDevice*,SoapySDRStream*){return 0;}
int SoapySDRDevice_readStream(SoapySDRDevice* p,SoapySDRStream*,void*const* buffers,size_t requested,int*,long long*,long){if(fault==2)return -3;if(fault==4)return int(requested+1);auto s=reinterpret_cast<State*>(p);if(s->pos>=fixture.size())return -3;size_t n=std::min(requested,(fixture.size()-s->pos)/2);std::copy_n(fixture.data()+s->pos,n*2,static_cast<float*>(buffers[0]));s->pos+=n*2;if(fault==3&&n)static_cast<float*>(buffers[0])[0]=std::numeric_limits<float>::infinity();return int(n);}
