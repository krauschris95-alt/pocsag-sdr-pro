#include "dsp.h"
#include <fstream>
#include <iostream>
#include <iomanip>
int main(int argc,char** argv){
 if(argc<2)return 2;std::string mode=argc>2?argv[2]:"ALPHA";std::ifstream in(argv[1],std::ios::binary);if(!in)return 2;
 pocsag::Demodulator dsp([&](pocsag::Message m){m=pocsag::interpret(std::move(m),mode);if((m.type=="TONE"||m.type=="DIAGNOSTIC") && !(argc>3 && std::string(argv[3])=="ALL"))return;std::cout<<m.ric<<'\t'<<m.function<<'\t'<<m.baud<<'\t'<<m.type<<'\t';for(unsigned char c:m.text)std::cout<<std::hex<<std::setfill('0')<<std::setw(2)<<unsigned(c);std::cout<<std::dec<<'\n';});
 std::vector<unsigned char>b(16384);while(in){in.read(reinterpret_cast<char*>(b.data()),b.size());dsp.cu8(std::span(b).first(in.gcount()));}dsp.finish();
}

