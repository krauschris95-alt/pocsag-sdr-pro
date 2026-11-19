#include "flexbridge.h"
#include <QByteArray>
#include <stdexcept>
#include <iostream>
using namespace paging;
static void require(bool yes,const char* why){if(!yes)throw std::runtime_error(why);}
int main(){
 auto valid=FlexBridge::parseValidatedJson(R"({"timestamp":"2026-11-19 11:00:00","baud":1600,"level":2,"phase":"A","cycle":1,"frame":12,"capcode":1234567,"addr_type":"S","msg_type":"alphanumeric","message":"INTERVENTION DE TEST","k_ok":true})");
 require(valid.has_value(),"valid FLEX_NEXT page");
 require(valid->address==1234567&&valid->baud==1600&&valid->levels==2,"FLEX metadata");
 require(valid->text=="INTERVENTION DE TEST"&&valid->type=="alphanumeric","FLEX message");
 auto large=FlexBridge::parseValidatedJson(R"({"baud":3200,"level":4,"capcode":4294967296,"msg_type":"numeric","message":"123456"})");
 require(large&&large->address==4294967296ULL,"FLEX address must not truncate to POCSAG 21-bit range");
 require(!FlexBridge::parseValidatedJson(R"({"baud":3200,"level":4,"capcode":123,"msg_type":"numeric","message":"123","k_ok":false})"),"reject invalid checksum");
 require(!FlexBridge::parseValidatedJson(R"({"baud":3200,"level":4,"capcode":123,"msg_type":"numeric","message":"123","sig_ok":false})"),"reject invalid signature");
 require(!FlexBridge::parseValidatedJson(R"({"baud":1600,"level":2,"capcode":123,"msg_type":"numeric","message":""})"),"reject empty numeric");
 require(!FlexBridge::parseValidatedJson(R"({"baud":2400,"level":2,"capcode":123,"msg_type":"numeric","message":"123"})"),"reject unsupported baud");
 require(!FlexBridge::parseValidatedJson(R"({"baud":1600,"level":2,"capcode":4297068543,"msg_type":"numeric","message":"123"})"),"reject invalid address");
 require(!FlexBridge::parseValidatedJson(R"({"baud":1600,"level":2,"capcode":123,"msg_type":"biw_date","message":"fake"})"),"no system BIW as user message");
 require(!FlexBridge::parseValidatedJson("garbage"),"reject non-JSON");
 std::cout<<"PASS: FLEX_NEXT validated JSON parsing, long addresses, integrity rejection, no false messages\n";
}
