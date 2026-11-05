#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
namespace pocsag {
constexpr uint32_t Sync=0x7cd215d8, Idle=0x7a89c197;
uint32_t encode(uint32_t information);
int correct(uint32_t& word);
struct Message {
 uint32_t ric; int function; int baud; std::string type,text; int corrected=0;
 std::string alpha,numeric,rawBits,status="validated";
 bool alphaValid=true;
 bool complete=true;
 std::string termination="boundary";
 std::string rawWords;
};
// Interpretation is a user/network policy, not a meaning encoded by the function bits.
Message interpret(Message message,const std::string& mode);
class Decoder {
public:
 struct Statistics {uint64_t preambles=0,syncs=0,words=0,rejected=0,correctedBits=0;};
 const Statistics& statistics()const{return statistics_;}
 explicit Decoder(int baud, std::function<void(Message)> output):baud_(baud),output_(std::move(output)){}
 void bit(bool value);
 void finish(bool complete=true,const std::string& reason="boundary");
 void interrupt(const std::string& reason);
 void reset();
 void discard(){active_=false;payload_.clear();corrections_=0;}
private:
 int baud_,count_=0,slot_=0,corrections_=0; uint32_t shift_=0; bool locked_=false,inverted_=false,active_=false;
 uint32_t ric_=0; int function_=0; std::vector<bool> payload_;
 std::function<void(Message)> output_;
 std::string rawWords_;
 void word(uint32_t value);
 Statistics statistics_;int alternations_=0;bool lastBit_=false;
};
}
