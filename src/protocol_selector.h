#pragma once

#include <chrono>
#include <optional>

// Protocol arbitration only: this does NOT decode FLEX or ReFLEX.
// A protocol decoder must independently validate its own frame (including
// applicable synchronization, FEC and integrity checks) before reporting it.
namespace paging {

enum class Protocol { Unknown, Pocsag, Flex, Reflex };
enum class SelectionMode { Auto, Pocsag, Flex, Reflex };

constexpr Protocol forcedProtocol(SelectionMode mode) {
 switch (mode) {
 case SelectionMode::Pocsag: return Protocol::Pocsag;
 case SelectionMode::Flex: return Protocol::Flex;
 case SelectionMode::Reflex: return Protocol::Reflex;
 case SelectionMode::Auto: return Protocol::Unknown;
 }
 return Protocol::Unknown;
}

class ProtocolSelector {
public:
 using Clock = std::chrono::steady_clock;
 using Time = Clock::time_point;
 struct Policy {
  // Multiple verified frames are required before AUTO announces a protocol.
  unsigned initialConfirmations = 2;
  unsigned switchConfirmations = 3;
  std::chrono::milliseconds confirmationWindow{8000};
  // A current protocol is not replaced while its validated frames keep arriving.
  std::chrono::milliseconds activeHold{12000};
 };

 ProtocolSelector() = default;
 explicit ProtocolSelector(Policy policy): policy_(policy) {}

 Protocol selected() const { return selected_; }
 SelectionMode mode() const { return mode_; }

 void setMode(SelectionMode mode) {
  mode_ = mode;
  reset();
 }

 void reset() {
  selected_ = forcedProtocol(mode_);
  lastCurrent_.reset();
  candidate_ = Protocol::Unknown;
  candidateLast_.reset();
  candidateCount_ = 0;
 }

 // Returns true only if the displayed selection changes. A detector must not
 // call this for mere RF energy, preamble guesses, or unverified sync words.
 bool validatedFrame(Protocol protocol, Time now) {
  if (mode_ != SelectionMode::Auto || protocol == Protocol::Unknown)
   return false;
  if (protocol == selected_) {
   lastCurrent_ = now;
   clearCandidate();
   return false;
  }
  if (protocol != candidate_ || !candidateLast_ || now < *candidateLast_ ||
      now - *candidateLast_ > policy_.confirmationWindow) {
   candidate_ = protocol;
   candidateCount_ = 0;
  }
  candidateLast_ = now;
  ++candidateCount_;
  const auto threshold = selected_ == Protocol::Unknown
      ? policy_.initialConfirmations : policy_.switchConfirmations;
  const bool oldProtocolStillActive = selected_ != Protocol::Unknown &&
      lastCurrent_ && now >= *lastCurrent_ &&
      now - *lastCurrent_ < policy_.activeHold;
  if (candidateCount_ < (threshold == 0 ? 1u : threshold) || oldProtocolStillActive)
   return false;
  selected_ = protocol;
  lastCurrent_ = now;
  clearCandidate();
  return true;
 }

private:
 void clearCandidate() {
  candidate_ = Protocol::Unknown;
  candidateLast_.reset();
  candidateCount_ = 0;
 }
 Policy policy_;
 SelectionMode mode_ = SelectionMode::Auto;
 Protocol selected_ = Protocol::Unknown;
 Protocol candidate_ = Protocol::Unknown;
 unsigned candidateCount_ = 0;
 std::optional<Time> lastCurrent_, candidateLast_;
};

} // namespace paging
