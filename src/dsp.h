#pragma once
#include "decoder.h"
#include <complex>
#include <memory>
#include <span>
#include <array>
#include <numbers>
namespace pocsag {
class Demodulator {
public:
 explicit Demodulator(std::function<void(Message)> output,int baud=0,double rate=240000,double channelBandwidth=21000):selected_(baud),rate_(rate){
  for(int b:{512,1200,2400})decoders_.push_back(std::make_unique<Decoder>(b,output));
  double cutoff=channelBandwidth/2;double total=0;for(int n=0;n<63;++n){double t=n-31;double v=t==0?2*cutoff/rate:sin(2*std::numbers::pi*cutoff*t/rate)/(std::numbers::pi*t);taps_[n]=v*(.5-.5*cos(2*std::numbers::pi*n/62));total+=taps_[n];}for(auto& t:taps_)t/=total;
 }
 void cu8(std::span<const unsigned char> bytes){
  const int bauds[]={512,1200,2400};
  for(size_t i=0;i+1<bytes.size();i+=2){history_[cursor_]=std::complex<float>((bytes[i]-127.5f)/128,(bytes[i+1]-127.5f)/128);cursor_=(cursor_+1)%63;if(++decimation_<5)continue;decimation_=0;std::complex<float>filtered{};for(int j=0;j<63;++j)filtered+=history_[(cursor_+j)%63]*float(taps_[j]);auto z=filtered*std::conj(previous_);previous_=filtered;double value=std::atan2(z.imag(),z.real());dc_+=.00025*(value-dc_);value-=dc_;
   for(int k=0;k<3;++k){if(selected_&&selected_!=bauds[k])continue;auto& c=clocks_[k];c.filtered+=.6*(value-c.filtered);bool sign=c.filtered>=0;if(sign!=c.previous){double error=c.phase<.5?c.phase:c.phase-1;c.phase-=error*.08;}c.previous=sign;c.sum+=c.filtered;c.phase+=bauds[k]/(rate_/5);if(c.phase>=1){c.phase-=1;decoders_[k]->bit(c.sum>=0);c.sum=0;}}
  }
 }
 void finish(){for(auto& d:decoders_)d->discard();}
 void reset(){for(auto& d:decoders_)d->reset();clocks_={};history_={};cursor_=decimation_=0;previous_={1,0};dc_=0;}
private:
 struct Clock{double phase=0,sum=0,filtered=0;bool previous=false;};std::array<Clock,3>clocks_{};
 int selected_,cursor_=0,decimation_=0;double rate_,dc_=0;std::array<double,63>taps_{};std::array<std::complex<float>,63>history_{};std::complex<float>previous_{1,0};std::vector<std::unique_ptr<Decoder>>decoders_;
};
}
