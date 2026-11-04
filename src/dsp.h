#pragma once
#include "decoder.h"
#include <complex>
#include <memory>
#include <span>
#include <array>
namespace pocsag {
class Demodulator {
public:
 explicit Demodulator(std::function<void(Message)> output,int baud=0,double rate=240000):selected_(baud),rate_(rate){for(int b:{512,1200,2400})decoders_.push_back(std::make_unique<Decoder>(b,output));}
 void cu8(std::span<const unsigned char> bytes){
  const int bauds[]={512,1200,2400};
  for(size_t i=0;i+1<bytes.size();i+=2){std::complex<float>x((bytes[i]-127.5f)/128,(bytes[i+1]-127.5f)/128);filtered_+=.22f*(x-filtered_);auto z=filtered_*std::conj(previous_);previous_=filtered_;double value=std::atan2(z.imag(),z.real());dc_+=.0001*(value-dc_);value-=dc_;
   for(int k=0;k<3;++k){if(selected_&&selected_!=bauds[k])continue;auto& c=clocks_[k];bool sign=value>=0;if(sign!=c.previous){double error=c.phase<.5?c.phase:c.phase-1;c.phase-=error*.08;}c.previous=sign;c.sum+=value;c.phase+=bauds[k]/rate_;if(c.phase>=1){c.phase-=1;decoders_[k]->bit(c.sum>=0);c.sum=0;}}
  }
 }
 void finish(){for(auto& d:decoders_)d->finish();}
 void reset(){for(auto& d:decoders_)d->reset();clocks_={};filtered_={};previous_={1,0};dc_=0;}
private:
 struct Clock{double phase=0,sum=0;bool previous=false;};std::array<Clock,3>clocks_{};
 int selected_;double rate_,dc_=0;std::complex<float>filtered_{},previous_{1,0};std::vector<std::unique_ptr<Decoder>>decoders_;
};
}
