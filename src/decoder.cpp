#include "decoder.h"
#include <bit>
#include <cstdio>
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
void Decoder::finish(bool complete,const std::string& reason){
 if(!active_)return;
 Message m{ric_,function_,baud_,payload_.empty()?"TONE":"ALPHA","",corrections_};
 m.rawWords=rawWords_;
 m.complete=complete;m.termination=reason;
 for(bool b:payload_)m.rawBits+=b?'1':'0';
 bool ended=false;
 for(size_t i=0;i+7<=payload_.size();i+=7){
  unsigned c=0;for(int j=0;j<7;++j)c|=unsigned(payload_[i+j])<<j;
  if(c==0||c==3||c==4){ended=true;continue;}
  if(ended){if(c!=127)m.alphaValid=false;continue;}
  if(c>=32&&c<127)m.alpha+=char(c);
  else if(c==9||c==10||c==13)m.alpha+=char(c);
  else{m.alphaValid=false;m.alpha+="<0x";const char*hex="0123456789ABCDEF";m.alpha+=hex[c>>4];m.alpha+=hex[c&15];m.alpha+='>';}
 }
 // Reversed-on-air numeric nibbles, CCIR 584 extended symbols.
 const char* table="0123456789.U -][";
 for(size_t i=0;i+4<=payload_.size();i+=4){unsigned c=0;for(int j=0;j<4;++j)c|=unsigned(payload_[i+j])<<j;m.numeric+=table[c];}
 while(!m.numeric.empty()&&m.numeric.back()==' ')m.numeric.pop_back();
 if(m.type!="TONE")m=interpret(std::move(m),"ALPHA");
 output_(std::move(m));discard();
}
Message interpret(Message m,const std::string& mode){
 if(!m.complete){m.type="INCOMPLETE";m.text=m.alpha;m.status="Incomplete payload: "+m.termination;return m;}
 if(mode=="TONE"){m.type="TONE";m.text="";m.status="Configured TONE policy; payload retained";return m;}
 if(m.rawBits.empty())return m;
 if(mode=="NUMERIC"){m.type="NUMERIC";m.text=m.numeric;m.status="BCH valid; NUMERIC selected by configured policy (not inferred from function)";}
 else if(mode=="ALPHA"){
  m.type=m.alphaValid&&!m.alpha.empty()?"ALPHA":"DIAGNOSTIC";m.text=m.alpha;
  m.status=m.type=="ALPHA"?"validated / alpha selected":"BCH valid; unsupported controls or empty alpha";
 }else{
  // Every four-bit value has a numeric representation. Printable ASCII or a
  // decimal-looking candidate cannot prove the sender's application encoding.
  // Only an explicit network/capcode policy can resolve that ambiguity safely.
  m.type="AMBIGUOUS";m.text=m.alpha.empty()?m.numeric:m.alpha;
  m.status=m.alphaValid?"BCH valid; ALPHA and NUMERIC candidates; sender encoding unknown":"BCH valid; unsupported ALPHA controls; NUMERIC or application/binary data possible";
 }return m;
}
void Decoder::interrupt(const std::string& reason){
 if(active_&&!payload_.empty())finish(false,reason);else discard();
}
void Decoder::bit(bool b){
 alternations_=b!=lastBit_?alternations_+1:0;lastBit_=b;if(alternations_==575)++statistics_.preambles;
 shift_=(shift_<<1)|unsigned(b);
 if(!locked_){
  if(shift_==Sync||shift_==~Sync){++statistics_.syncs;inverted_=shift_==~Sync;locked_=true;count_=slot_=0;}
  return;
 }
 if(++count_<32)return;count_=0;auto v=inverted_?~shift_:shift_;
 if(slot_==16){if(std::popcount(v^Sync)<=2){slot_=0;return;}interrupt("batch sync lost");locked_=false;return;}
 word(v);++slot_;
}
void Decoder::word(uint32_t v){
 char hex[10];std::snprintf(hex,sizeof(hex),"%08X ",v);
 if(active_)rawWords_+=hex;
 ++statistics_.words;int n=correct(v);if(n<0){++statistics_.rejected;interrupt("uncorrectable BCH/parity word");return;}statistics_.correctedBits+=n;
 if(v==Idle){finish();return;}
 if(!(v&0x80000000)){finish();rawWords_=hex;ric_=((v>>13)<<3)|(slot_/2);function_=(v>>11)&3;active_=true;corrections_=n;return;}
 if(active_){if(payload_.size()>=65520){interrupt("payload assembly safety bound");return;}corrections_+=n;for(int i=30;i>=11;--i)payload_.push_back((v>>i)&1);}
}
}
