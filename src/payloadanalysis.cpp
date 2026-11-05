#include "payloadanalysis.h"
#include <cctype>
namespace pocsag {
namespace {
bool utf8Text(const std::string& bytes){for(size_t i=0;i<bytes.size();){unsigned c=(unsigned char)bytes[i++];if(c<128){if((c<32&&c!=9&&c!=10&&c!=13)||c==127)return false;continue;}int n;unsigned value,min;if(c>=0xc2&&c<=0xdf){n=1;value=c&31;min=128;}else if(c>=0xe0&&c<=0xef){n=2;value=c&15;min=2048;}else if(c>=0xf0&&c<=0xf4){n=3;value=c&7;min=65536;}else return false;if(i+n>bytes.size())return false;while(n--){unsigned b=(unsigned char)bytes[i++];if((b&0xc0)!=0x80)return false;value=(value<<6)|(b&63);}if(value<min||value>0x10ffff||(value>=0xd800&&value<=0xdfff)||(value>=0x80&&value<0xa0))return false;}return true;}
bool decode64(const std::string& text,std::string& bytes){if(text.size()<24||text.size()%4)return false;const std::string alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";for(size_t i=0;i<text.size();i+=4){unsigned v=0;int padding=0;for(int j=0;j<4;++j){char c=text[i+j];if(c=='='){if(i+4!=text.size()||j<2)return false;++padding;v<<=6;}else{if(padding)return false;auto n=alphabet.find(c);if(n==std::string::npos)return false;v=(v<<6)|unsigned(n);}}if(padding>2||(padding==2&&(v&0xffff))||(padding==1&&(v&0xff)))return false;bytes+=char(v>>16);if(padding<2)bytes+=char(v>>8);if(!padding)bytes+=char(v);}return bytes.size()>=16;}
}
PayloadAssessment assessAlpha(const std::string& text){PayloadAssessment out;std::string bytes;if(decode64(text,bytes)){out.base64=true;out.binary=!utf8Text(bytes);out.description=out.binary?"Base64 canonique ; octets non textuels ; format applicatif inconnu":"Base64 canonique possible ; octets UTF-8 textuels ; contenu original conservé";if(bytes.starts_with(std::string("\x1f\x8b",2)))out.description+=" ; signature compatible gzip (non décompressé)";else if(bytes.starts_with("PK\x03\x04"))out.description+=" ; signature compatible ZIP (non extrait)";return out;}
 int letters=0,spaces=0,symbols=0;bool printable=true;for(unsigned char c:text){if(std::isalpha(c))++letters;else if(std::isspace(c))++spaces;else if(!std::isdigit(c))++symbols;if((c<32&&c!=9&&c!=10&&c!=13)||c>=127)printable=false;}
 out.plausibleAlpha=printable&&text.find("<0x")==std::string::npos&&letters>=3&&symbols*3<=int(text.size())&&(spaces>0||text.size()<=24);
 out.description=out.plausibleAlpha?"Texte ALPHA plausible ; sélection heuristique, alternative NUMERIC conservée":"Charge utile non interprétée ; ALPHA/NUMERIC ou format applicatif inconnu";return out;
}
}
