#include "payloadanalysis.h"
#include <cstdint>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::string base64(const std::string& bytes){
 static constexpr char abc[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 std::string out;
 for(std::size_t i=0;i<bytes.size();i+=3){
  const unsigned a=static_cast<unsigned char>(bytes[i]);
  const unsigned b=i+1<bytes.size()?static_cast<unsigned char>(bytes[i+1]):0;
  const unsigned c=i+2<bytes.size()?static_cast<unsigned char>(bytes[i+2]):0;
  out+=abc[a>>2];
  out+=abc[((a&3)<<4)|(b>>4)];
  out+=i+1<bytes.size()?abc[((b&15)<<2)|(c>>6)]:'=';
  out+=i+2<bytes.size()?abc[c&63]:'=';
 }
 return out;
}
int main(){
 const auto canonical=pocsag::inspectBase64("AAECAwQFBgcICQoLDA0ODxAREhMUFRYX",4);
 require(canonical.recognized,"canonical Base64 should decode");
 require(canonical.byteCount==24,"byte count should reflect full payload");
 require(canonical.hexPreview=="00 01 02 03","hex preview must preserve exact bytes");
 require(canonical.asciiPreview=="....","ASCII preview must mask nonprintable bytes");
 require(canonical.truncated,"preview should be marked truncated");
 require(canonical.signature=="Format binaire inconnu","unknown bytes are not proof of encryption");
 require(canonical.entropyBitsPerByte>=0&&canonical.entropyBitsPerByte<=8,"entropy bound");
 const auto ascii=pocsag::inspectBase64(base64("HELLO FROM THE TEST"));
 require(ascii.recognized&&ascii.byteCount==19,"text payload should decode");
 require(ascii.signature=="Texte ASCII possible","ASCII hint only");
 require(ascii.asciiPreview=="HELLO FROM THE TEST","ASCII preview");
 const auto gzip=pocsag::inspectBase64(base64(std::string("\x1f\x8b",2)+std::string(18,'X')));
 require(gzip.recognized&&gzip.signature=="Signature gzip possible","gzip magic");
 const auto zip=pocsag::inspectBase64(base64(std::string("PK\x03\x04",4)+std::string(18,'X')));
 require(zip.recognized&&zip.signature=="Signature ZIP possible","zip magic");
 const auto invalid=pocsag::inspectBase64("AAECAwQFBgcICQoLDA0ODxAREhMUFRY!");
 require(!invalid.recognized,"invalid alphabet must be rejected");
 require(!pocsag::inspectBase64(std::string(28,'A')+"AB==").recognized,"noncanonical padding must be rejected");
 require(!pocsag::inspectBase64("SGVsbG8=").recognized,"short generic text is not confidently Base64");
 require(!pocsag::inspectBase64("AAECAwQFBgcICQoLDA0ODxAREhMUFRY").recognized,"truncated Base64 must be rejected");
 const auto old=pocsag::assessAlpha("AAECAwQFBgcICQoLDA0ODxAREhMUFRYX");
 require(old.base64&&old.binary,"existing ambiguity classifier must remain unchanged");
 std::cout<<"PASS: canonical Base64, hex/ASCII preview, length, truncation, entropy, signatures, malformed inputs, unchanged ambiguity classification\n";
}
