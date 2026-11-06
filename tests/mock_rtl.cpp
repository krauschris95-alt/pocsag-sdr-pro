// Fault-injection driver for automated tests only. Never included in the application ZIP.
#include <atomic>
#include <cstring>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#define API extern "C" __declspec(dllexport)
static std::vector<unsigned char> fixture;static std::atomic_bool cancelled=false;static int opens=0;static unsigned frequency=0,rate=0;
static std::atomic_bool paused=false;static std::atomic<unsigned> rejectedFrequency=0;
API void mock_reject_frequency(unsigned hz){rejectedFrequency=hz;}
API unsigned rtlsdr_get_center_freq(void*){return frequency;}
API void mock_pause(int value){paused=value!=0;}
API void mock_fixture(const char* path){std::ifstream f(path,std::ios::binary);fixture.assign(std::istreambuf_iterator<char>(f),{});}
API unsigned mock_frequency(){return frequency;}API unsigned mock_rate(){return rate;}
API unsigned rtlsdr_get_device_count(){return 1;}API const char* rtlsdr_get_device_name(unsigned){return "TEST DRIVER — NOT HARDWARE";}
API int rtlsdr_get_device_usb_strings(unsigned,char* m,char* p,char* s){std::strcpy(m,"Test");std::strcpy(p,"Fault injection");std::strcpy(s,"TEST-ONLY");return 0;}
API int rtlsdr_open(void** d,unsigned){*d=reinterpret_cast<void*>(1);++opens;return 0;}API int rtlsdr_close(void*){return 0;}
API int rtlsdr_set_sample_rate(void*,unsigned v){rate=v;return 0;}API int rtlsdr_set_center_freq(void*,unsigned v){if(v==rejectedFrequency)return -1;frequency=v;return 0;}
API int rtlsdr_set_freq_correction(void*,int){return -2;}API int rtlsdr_set_tuner_gain_mode(void*,int){return 0;}API int rtlsdr_set_tuner_gain(void*,int){return 0;}API int rtlsdr_set_tuner_bandwidth(void*,unsigned){return 0;}API int rtlsdr_reset_buffer(void*){return 0;}API int rtlsdr_get_tuner_gains(void*,int* values){if(values){values[0]=0;values[1]=300;values[2]=490;}return 3;}
API int rtlsdr_cancel_async(void*){cancelled=true;return 0;}
API int rtlsdr_read_async(void*,void(*callback)(unsigned char*,unsigned,void*),void* context,unsigned,unsigned){cancelled=false;do{if(paused){std::this_thread::sleep_for(std::chrono::milliseconds(10));continue;}for(size_t n=0;n<fixture.size()&&!cancelled;n+=16384){callback(fixture.data()+n,unsigned(std::min(size_t(16384),fixture.size()-n)),context);std::this_thread::sleep_for(std::chrono::milliseconds(2));}if(opens==1)return -1;}while(!cancelled);return 0;}
