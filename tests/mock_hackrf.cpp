// Signed CS8 and USB fault simulator, never included in a release package.
#include <atomic>
#include <thread>
#include <fstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm>
#define API extern "C" __declspec(dllexport)
struct List{char** serials;int* boards;int* indices;int count;void** devices;int usbCount;};
struct Transfer{void* device;unsigned char* buffer;int length,valid;void* rx;void* tx;};
struct Session{std::atomic_bool running=false;std::thread worker;};
static std::vector<unsigned char> fixture;static std::atomic<int> fault=0;static std::atomic<unsigned> configuredRate=0,configuredFrequency=0;
API void mock_fixture(const char* path){std::ifstream file(path,std::ios::binary);fixture.assign(std::istreambuf_iterator<char>(file),{});}
API void mock_fault(int value){fault=value;}
API unsigned mock_frequency(){return configuredFrequency;}
API int hackrf_init(){return 0;}API int hackrf_exit(){return 0;}
API List* hackrf_device_list(){static char text[]="HACK-SIMULATED";static char* serials[]{text};static int boards[]{0x6089},indices[]{0};static List list{serials,boards,indices,1,nullptr,1};return &list;}
API void hackrf_device_list_free(List*){}
API const char* hackrf_usb_board_id_name(int){return "HackRF One (SIMULATED)";}
API int hackrf_open_by_serial(const char* serial,void** out){if(std::string(serial)!="HACK-SIMULATED")return -5;*out=new Session;return 0;}
API int hackrf_stop_rx(void* p){auto s=static_cast<Session*>(p);s->running=false;if(s->worker.joinable())s->worker.join();return 0;}
API int hackrf_close(void* p){hackrf_stop_rx(p);delete static_cast<Session*>(p);return 0;}
API int hackrf_set_sample_rate(void*,double r){configuredRate=unsigned(r);return r==2000000?0:-2;}
API int hackrf_set_freq(void*,unsigned long long f){configuredFrequency=unsigned(f);return fault==3?-2:0;}
API int hackrf_set_lna_gain(void*,unsigned){return 0;}API int hackrf_set_vga_gain(void*,unsigned){return 0;}
API unsigned hackrf_compute_baseband_filter_bw(unsigned n){return n;}
API int hackrf_set_baseband_filter_bandwidth(void*,unsigned){return 0;}
API int hackrf_is_streaming(void* p){return static_cast<Session*>(p)->running;}
API int hackrf_start_rx(void* p,int(*callback)(Transfer*),void* context){if(fixture.empty())return -2;auto s=static_cast<Session*>(p);s->running=true;s->worker=std::thread([=]{double at=0;std::vector<unsigned char> data(32768);while(s->running){if(fault==1){s->running=false;break;}if(fault==2){std::this_thread::sleep_for(std::chrono::milliseconds(10));continue;}for(size_t i=0;i<data.size()/2;++i){size_t sample=size_t(at);double fraction=at-sample;for(int channel=0;channel<2;++channel){int a=fixture[(sample*2+channel)%fixture.size()],b=fixture[((sample+1)*2+channel)%fixture.size()];data[i*2+channel]=unsigned(std::clamp(int(std::lround(a+(b-a)*fraction)),0,255))^0x80;}at+=240000./configuredRate;if(at>=fixture.size()/2.)at-=fixture.size()/2.;}Transfer t{s,data.data(),int(data.size()),int(data.size()),context,nullptr};if(callback(&t)!=0){s->running=false;break;}std::this_thread::sleep_for(std::chrono::milliseconds(1));}});return 0;}
