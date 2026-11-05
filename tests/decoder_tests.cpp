#include "decoder.h"
#include "dsp.h"
#include "payloadanalysis.h"
#include <cmath>
#include <numbers>
#include <iostream>
#include <stdexcept>
using namespace pocsag;
void require(bool b){if(!b)throw std::runtime_error("test failed");}
void feed(Decoder& d,uint32_t w,bool inv=false){for(int i=31;i>=0;--i)d.bit(bool((w>>i)&1)^inv);}
int main(){
 require(encode(Idle>>11)==Idle);
 {auto bad=encode(0x123456)^7;require(correct(bad)==-1);}
 for(int i=0;i<32;++i)for(int j=i;j<32;++j){auto w=encode(0x123456)^(1u<<i);if(j!=i)w^=1u<<j;require(correct(w)==(j==i?1:2));require(w==encode(0x123456));}
 for(int baud:{512,1200,2400})for(bool inv:{false,true}){
  std::vector<Message> out;Decoder d(baud,[&](Message m){out.push_back(m);});
  int function=baud==512?0:3;feed(d,Sync,inv);feed(d,encode((760832u>>3)<<2|function),inv);
  std::vector<bool> bits;for(char c:std::string("BONJOUR"))for(int i=0;i<7;++i)bits.push_back((c>>i)&1);
  for(int k=0;k<4;++k){uint32_t info=1u<<20;for(int j=0;j<20;++j)if(k*20+j<int(bits.size())&&bits[k*20+j])info|=1u<<(19-j);feed(d,encode(info),inv);}
  feed(d,Idle,inv);require(out.size()==1&&out[0].text=="BONJOUR"&&out[0].ric==760832&&out[0].baud==baud&&out[0].function==function);require(interpret(out[0],"AUTO").type=="ALPHA");
  feed(d,encode((122120u>>3)<<2),inv);feed(d,Idle,inv);require(out.back().type=="TONE");
 }
 for(int function=0;function<4;++function){std::vector<Message> out;Decoder d(1200,[&](Message m){out.push_back(m);});feed(d,Sync);for(int i=0;i<14;++i)feed(d,Idle);feed(d,encode((2097151u>>3)<<2|function));feed(d,Idle);require(out.size()==1&&out[0].ric==2097151&&out[0].function==function);}
 {
  std::vector<Message> out;Decoder d(1200,[&](Message m){out.push_back(m);});feed(d,Sync);for(int i=0;i<14;++i)feed(d,Idle);feed(d,encode((2097151u>>3)<<2|3));
  std::vector<bool> bits;for(char c:std::string("FRAGMENTATION"))for(int j=0;j<7;++j)bits.push_back((c>>j)&1);
  for(int k=0;k<6;++k){if(k==1)feed(d,Sync^3);uint32_t info=1u<<20;for(int j=0;j<20;++j)if(k*20+j<int(bits.size())&&bits[k*20+j])info|=1u<<(19-j);feed(d,encode(info));}feed(d,Idle);require(out.size()==1&&out[0].text=="FRAGMENTATION"&&out[0].ric==2097151);
 }
 {std::vector<Message> out;Decoder d(1200,[&](Message m){out.push_back(m);});feed(d,Sync);feed(d,encode(3));for(int i=1;i<16;++i)feed(d,encode(1u<<20));feed(d,0);d.finish();require(out.size()==1&&out[0].type=="INCOMPLETE"&&!out[0].complete);out.clear();feed(d,Sync);feed(d,encode(3));feed(d,Idle);require(out.size()==1);}
 for(int baud:{512,1200,2400})for(bool inv:{false,true}){
  std::vector<bool> bits;for(int i=0;i<576;++i)bits.push_back(i%2);
  auto word=[&](uint32_t w){for(int i=31;i>=0;--i)bits.push_back(bool((w>>i)&1)^inv);};
  word(Sync);word(encode((123456u>>3)<<2));uint32_t info=1u<<20;const int digits[]={1,2,3,4,5};for(int k=0;k<5;++k)for(int j=0;j<4;++j)if(digits[k]&(1<<j))info|=1u<<(19-(k*4+j));word(encode(info));for(int i=2;i<16;++i)word(Idle);
  std::vector<Message> out;Demodulator demod([&](Message m){out.push_back(interpret(std::move(m),"NUMERIC"));});std::vector<unsigned char> iq;double phase=0;size_t samples=size_t(std::ceil(bits.size()*240000.0/baud));
  for(size_t n=0;n<samples;++n){size_t bit=std::min(bits.size()-1,size_t(n*double(baud)/240000));phase+=(bits[bit]?1:-1)*2*std::numbers::pi*4500/240000;iq.push_back(static_cast<unsigned char>(128+100*cos(phase)));iq.push_back(static_cast<unsigned char>(128+100*sin(phase)));}
  for(size_t i=0;i<iq.size();i+=16384)demod.cu8(std::span(iq).subspan(i,std::min(size_t(16384),iq.size()-i)));demod.finish();require(out.size()==1&&out[0].text=="12345"&&out[0].ric==123456&&out[0].baud==baud);
 }
 // All numeric symbols, all functions/bauds, a batch-spanning payload, and two-bit corrections.
 for(int baud:{512,1200,2400})for(int function=0;function<4;++function){
  std::vector<Message> out;Decoder d(baud,[&](Message m){out.push_back(m);});feed(d,Sync);
  for(int i=0;i<14;++i)feed(d,Idle);feed(d,encode((123455u>>3)<<2|function));
  std::string expected="0123456789.U -][0123456789.U -][";std::vector<bool> bits;
  std::string alphabet="0123456789.U -][";for(char c:expected){unsigned v=alphabet.find(c);for(int j=0;j<4;++j)bits.push_back((v>>j)&1);}
  while(bits.size()%20){unsigned pad=12;for(int j=0;j<4;++j)bits.push_back((pad>>j)&1);}
  for(size_t k=0;k<bits.size()/20;++k){if(k==1)feed(d,Sync);uint32_t info=1u<<20;for(int j=0;j<20;++j)if(bits[k*20+j])info|=1u<<(19-j);feed(d,encode(info)^(k==2?3:0));}
  feed(d,Idle);require(out.size()==1);auto numeric=interpret(out[0],"NUMERIC");require(numeric.text==expected&&numeric.type=="NUMERIC"&&numeric.function==function&&numeric.baud==baud&&numeric.corrected==2);
  require(interpret(out[0],"AUTO").type=="AMBIGUOUS");require(interpret(out[0],"TONE").type=="TONE");require(!numeric.rawBits.empty());
 }
 {std::vector<Message> out;Decoder d(2400,[&](Message m){out.push_back(m);});feed(d,Sync);feed(d,encode(3));std::vector<bool> bits;for(unsigned c:{1u,2u,27u,127u,1u,2u,27u,127u})for(int j=0;j<7;++j)bits.push_back((c>>j)&1);while(bits.size()%20)bits.push_back(0);for(size_t k=0;k<bits.size()/20;++k){uint32_t info=1u<<20;for(int j=0;j<20;++j)if(bits[k*20+j])info|=1u<<(19-j);feed(d,encode(info));}feed(d,Idle);require(out.size()==1&&!out[0].alphaValid&&out[0].type=="DIAGNOSTIC"&&!out[0].rawWords.empty());require(interpret(out[0],"AUTO").type=="AMBIGUOUS");auto partial=out[0];partial.complete=false;partial.termination="uncorrectable word";require(interpret(partial,"NUMERIC").type=="INCOMPLETE");}
 {auto binary=assessAlpha("AAECAwQFBgcICQoLDA0ODxAREhMUFRYX");require(binary.base64&&binary.binary&&!binary.plausibleAlpha);require(!assessAlpha("AAECAwQFBgcICQoLDA0ODxAREhMUFRY!").base64);require(!assessAlpha("AAAAAAAAAAAAAAAAAAAAAB==").base64);require(assessAlpha("APPEL : intervention de test").plausibleAlpha);require(!assessAlpha("abcdefghijklmnopqrstuvwxyz0123456789!@").plausibleAlpha);Message m{42,0,2400,"ALPHA","",0};m.alpha="AAECAwQFBgcICQoLDA0ODxAREhMUFRYX";m.rawBits="1010";m.numeric="5";require(interpret(m,"AUTO").type=="AMBIGUOUS"&&interpret(m,"ALPHA").type=="AMBIGUOUS");m.alpha="APPEL : intervention de test";require(interpret(m,"AUTO").type=="ALPHA");}
 std::cout<<"PASS: BCH 528 error masks; 3 baud rates; both polarities; ALPHA/NUMERIC/TONE; RIC; function; synthetic CU8 FSK; extended numeric/all functions/batch split/2-bit BCH; binary ambiguity/incomplete\n";
}
