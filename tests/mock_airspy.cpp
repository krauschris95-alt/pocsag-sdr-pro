// Isolated ABI simulator. Never shipped as a radio driver.
#include <atomic>
#include <thread>
#include <vector>
#include <fstream>
#include <chrono>
#include <cstdint>
#include <limits>
#define API extern "C" __declspec(dllexport)
struct Transfer {void* device;void* context;float* samples;int count;uint64_t dropped;int type;};
struct Session {std::atomic_bool running=false;std::thread worker;unsigned rate=0,frequency=0;};
static std::vector<float> fixture;static std::atomic<int> fault=0,closed=0;
API void mock_fixture(const char* path){std::ifstream input(path,std::ios::binary);std::vector<unsigned char> data(std::istreambuf_iterator<char>(input),{});fixture.clear();for(auto c:data)fixture.push_back((int(c)-127.5f)/128.f);}
API void mock_fault(int value){fault=value;}
API int mock_closed(){return closed;}
API int airspy_list_devices(uint64_t* output,int capacity){if(output&&capacity>0)output[0]=0x1234;if(output&&capacity>1)output[1]=0x5678;return 2;}
API int airspy_open_sn(void** output,uint64_t serial){if(serial!=0x1234&&serial!=0x5678)return -5;*output=new Session;return 0;}
API int airspy_stop_rx(void* handle){auto s=static_cast<Session*>(handle);s->running=false;if(s->worker.joinable())s->worker.join();return 0;}
API int airspy_close(void* handle){airspy_stop_rx(handle);delete static_cast<Session*>(handle);++closed;return 0;}
API int airspy_get_samplerates(void*,uint32_t* output,uint32_t count){if(!count)*output=2;else{if(count<2)return -2;output[0]=240000;output[1]=2500000;}return 0;}
API int airspy_set_samplerate(void* handle,uint32_t value){if(value!=240000&&value!=2500000)return -2;static_cast<Session*>(handle)->rate=value;return 0;}
API int airspy_set_freq(void* handle,uint32_t value){if(fault==4)return -2;static_cast<Session*>(handle)->frequency=value;return 0;}
API int airspy_set_sample_type(void*,int value){return value==0?0:-2;}
API int airspy_set_lna_agc(void*,uint8_t){return 0;}
API int airspy_set_mixer_agc(void*,uint8_t){return 0;}
API int airspy_set_linearity_gain(void*,uint8_t value){return value<=21?0:-2;}
API int airspy_is_streaming(void* handle){return static_cast<Session*>(handle)->running?1:0;}
API int airspy_start_rx(void* handle,int(*callback)(Transfer*),void* context){auto s=static_cast<Session*>(handle);if(s->running||fixture.empty())return -2;s->running=true;s->worker=std::thread([=]{size_t pos=0;while(s->running){if(fault==1){s->running=false;break;}if(fault==2){std::this_thread::sleep_for(std::chrono::milliseconds(10));continue;}int count=int(std::min(size_t(8192),fixture.size()-pos));if(count%2)--count;if(!count){pos=0;continue;}std::vector<float> data(fixture.begin()+pos,fixture.begin()+pos+count);if(fault==3)data[0]=std::numeric_limits<float>::quiet_NaN();Transfer t{s,context,data.data(),count/2,0,0};if(callback(&t)!=0){s->running=false;break;}pos+=count;if(pos>=fixture.size())pos=0;std::this_thread::sleep_for(std::chrono::milliseconds(1));}});return 0;}
API int airspyhf_list_devices(uint64_t* out,int n){return airspy_list_devices(out,n);}
API int airspyhf_open_sn(void** out,uint64_t serial){return airspy_open_sn(out,serial);}
API int airspyhf_close(void* h){return airspy_close(h);}
API int airspyhf_stop(void* h){return airspy_stop_rx(h);}
API int airspyhf_get_samplerates(void* h,uint32_t* out,uint32_t n){return airspy_get_samplerates(h,out,n);}
API int airspyhf_set_samplerate(void* h,uint32_t n){return airspy_set_samplerate(h,n);}
API int airspyhf_set_freq(void* h,uint32_t n){return airspy_set_freq(h,n);}
API int airspyhf_set_hf_agc(void*,uint8_t){return 0;}
API int airspyhf_is_streaming(void* h){return airspy_is_streaming(h);}
// HF transfer is the prefix of Transfer, with no sample_type member.
API int airspyhf_start(void* h,int(*callback)(Transfer*),void* context){return airspy_start_rx(h,callback,context);}
