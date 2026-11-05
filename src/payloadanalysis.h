#pragma once
#include <string>
namespace pocsag {
struct PayloadAssessment {bool base64=false,binary=false,plausibleAlpha=false;std::string description;};
PayloadAssessment assessAlpha(const std::string& text);
}
