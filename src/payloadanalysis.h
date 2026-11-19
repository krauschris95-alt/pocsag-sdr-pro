#pragma once
#include <string>
#include <cstddef>
namespace pocsag {
struct PayloadAssessment {bool base64=false,binary=false,plausibleAlpha=false;std::string description;};
PayloadAssessment assessAlpha(const std::string& text);
// Local inspection of canonical Base64; this is not decryption or identification
// of an encryption algorithm. Never change the original radio message.
struct BinaryPayloadDetails {
 bool recognized=false;
 std::size_t byteCount=0;
 std::string hexPreview, asciiPreview, signature;
 double entropyBitsPerByte=0;
 bool truncated=false;
};
BinaryPayloadDetails inspectBase64(const std::string& text, std::size_t previewBytes=64);
}
