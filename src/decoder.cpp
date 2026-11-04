#include "decoder.h"
#include <bit>
namespace pocsag {
static uint32_t remainder(uint32_t v) { for(int i=30;i>=10;--i) if(v&(1u<<i)) v^=0x769u<<(i-10); return v&1023; }
uint32_t encode(uint32_t info) { uint32_t v=(info&0x1fffff)<<10; v|=remainder(v); return (v<<1)|(std::popcount(v)&1); }
static bool valid(uint32_t v){return remainder(v>>1)==0 && (std::popcount(v)&1)==0;}
int correct(uint32_t& v){
 if(valid(v))return 0;
 for(int i=0;i<32;++i){auto w=v^(1u<<i);if(valid(w)){v=w;return 1;}}
 for(int i=0;i<32;++i)for(int j=i+1;j<32;++j){auto w=v^(1u<<i)^(1u<<j);if(valid(w)){v=w;return 2;}}
 return -1;
}
void Decoder::reset(){count_=slot_=corrections_=0;shift_=0;locked_=inverted_=active_=false;payload_.clear();}
void Decoder::finish(){
 if(!active_)return;
 Message m{ric_,function_,baud_,payload_.empty()?"TONE":(function_==0?"NUMERIC":"ALPHA"),"",corrections_};
 if(m.type=="ALPHA"){
  for(size_t i=0;i+7<=payload_.size();i+=7){unsigned c=0;for(int j=0;j<7;++j)c|=unsigned(payload_[i+j])<<j;if(c==0||c==3||c==4)break;if(c>=32||c==10||c==13)m.text+=char(c);}
 }else if(m.type=="NUMERIC"){
  const char* table="0123456789*U -)(";
  for(size_t i=0;i+4<=payload_.size();i+=4){unsigned c=0;for(int j=0;j<4;++j)c|=unsigned(payload_[i+j])<<j;m.text+=table[c];}
  while(!m.text.empty()&&m.text.back()==' ')m.text.pop_back();
 }
 output_(std::move(m));active_=false;payload_.clear();corrections_=0;
}
void Decoder::bit(bool b){
 shift_=(shift_<<1)|unsigned(b);
 if(!locked_){
  if(shift_==Sync||shift_==~Sync){inverted_=shift_==~Sync;locked_=true;count_=slot_=0;}
  return;
 }
 if(++count_<32)return;count_=0;auto v=inverted_?~shift_:shift_;
 if(slot_==16){if(std::popcount(v^Sync)<=2){slot_=0;return;}finish();locked_=false;return;}
 word(v);++slot_;
}
void Decoder::word(uint32_t v){
 int n=correct(v);if(n<0){active_=false;payload_.clear();return;}
 if(v==Idle){finish();return;}
 if(!(v&0x80000000)){finish();ric_=((v>>13)<<3)|(slot_/2);function_=(v>>11)&3;active_=true;corrections_=n;return;}
 if(active_){corrections_+=n;for(int i=30;i>=11;--i)payload_.push_back((v>>i)&1);}
}
}
