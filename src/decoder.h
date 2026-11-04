#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
namespace pocsag {
constexpr uint32_t Sync=0x7cd215d8, Idle=0x7a89c197;
uint32_t encode(uint32_t information);
int correct(uint32_t& word);
struct Message { uint32_t ric; int function; int baud; std::string type,text; int corrected=0; };
class Decoder {
public:
 explicit Decoder(int baud, std::function<void(Message)> output):baud_(baud),output_(std::move(output)){}
 void bit(bool value);
 void finish();
 void reset();
private:
 int baud_,count_=0,slot_=0,corrections_=0; uint32_t shift_=0; bool locked_=false,inverted_=false,active_=false;
 uint32_t ric_=0; int function_=0; std::vector<bool> payload_;
 std::function<void(Message)> output_;
 void word(uint32_t value);
};
}
