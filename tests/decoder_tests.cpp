#include "decoder.h"
#include "dsp.h"
#include <cmath>
#include <numbers>
#include <iostream>
#include <stdexcept>
using namespace pocsag;
void require(bool b){if(!b)throw std::runtime_error("test failed");}
void feed(Decoder& d,uint32_t w,bool inv=false){for(int i=31;i>=0;--i)d.bit(bool((w>>i)&1)^inv);}
int main(){
 require(encode(Idle>>11)==Idle);
 for(int i=0;i<32;++i)for(int j=i;j<32;++j){auto w=encode(0x123456)^(1u<<i);if(j!=i)w^=1u<<j;require(correct(w)==(j==i?1:2));require(w==encode(0x123456));}
 for(int baud:{512,1200,2400})for(bool inv:{false,true}){
  std::vector<Message> out;Decoder d(baud,[&](Message m){out.push_back(m);});
  feed(d,Sync,inv);feed(d,encode((760832u>>3)<<2|3),inv);
  std::vector<bool> bits;for(char c:std::string("BONJOUR"))for(int i=0;i<7;++i)bits.push_back((c>>i)&1);
  for(int k=0;k<4;++k){uint32_t info=1u<<20;for(int j=0;j<20;++j)if(k*20+j<int(bits.size())&&bits[k*20+j])info|=1u<<(19-j);feed(d,encode(info),inv);}
  feed(d,Idle,inv);require(out.size()==1&&out[0].text=="BONJOUR"&&out[0].ric==760832&&out[0].baud==baud&&out[0].function==3);
  feed(d,encode((122120u>>3)<<2),inv);feed(d,Idle,inv);require(out.back().type=="TONE");
 }
 for(int baud:{512,1200,2400})for(bool inv:{false,true}){
  std::vector<bool> bits;for(int i=0;i<576;++i)bits.push_back(i%2);
  auto word=[&](uint32_t w){for(int i=31;i>=0;--i)bits.push_back(bool((w>>i)&1)^inv);};
  word(Sync);word(encode((123456u>>3)<<2));uint32_t info=1u<<20;const int digits[]={1,2,3,4,5};for(int k=0;k<5;++k)for(int j=0;j<4;++j)if(digits[k]&(1<<j))info|=1u<<(19-(k*4+j));word(encode(info));for(int i=2;i<16;++i)word(Idle);
  std::vector<Message> out;Demodulator demod([&](Message m){out.push_back(m);});std::vector<unsigned char> iq;double phase=0;size_t samples=size_t(std::ceil(bits.size()*240000.0/baud));
  for(size_t n=0;n<samples;++n){size_t bit=std::min(bits.size()-1,size_t(n*double(baud)/240000));phase+=(bits[bit]?1:-1)*2*std::numbers::pi*4500/240000;iq.push_back(static_cast<unsigned char>(128+100*cos(phase)));iq.push_back(static_cast<unsigned char>(128+100*sin(phase)));}
  for(size_t i=0;i<iq.size();i+=16384)demod.cu8(std::span(iq).subspan(i,std::min(size_t(16384),iq.size()-i)));demod.finish();require(out.size()==1&&out[0].text=="12345"&&out[0].ric==123456&&out[0].baud==baud);
 }
 std::cout<<"PASS: BCH 528 error masks; 3 baud rates; both polarities; ALPHA/NUMERIC/TONE; RIC; function; synthetic CU8 FSK end-to-end AUTO\n";
}
