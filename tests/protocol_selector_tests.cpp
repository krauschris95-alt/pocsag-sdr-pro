#include "protocol_selector.h"
#include <chrono>
#include <stdexcept>
#include <iostream>

using namespace paging;
using namespace std::chrono_literals;

void require(bool value, const char* reason) {
 if (!value) throw std::runtime_error(reason);
}

int main() {
 using T = ProtocolSelector::Time;
 const T t{};
 ProtocolSelector selector;
 require(selector.selected() == Protocol::Unknown, "AUTO must start unknown");
 require(!selector.validatedFrame(Protocol::Pocsag, t), "one POCSAG frame must not lock");
 require(selector.validatedFrame(Protocol::Pocsag, t + 1s), "two POCSAG frames must lock");
 require(selector.selected() == Protocol::Pocsag, "POCSAG selected");
 require(!selector.validatedFrame(Protocol::Flex, t + 2s), "one FLEX frame cannot steal lock");
 require(!selector.validatedFrame(Protocol::Flex, t + 3s), "two FLEX frames cannot steal lock");
 require(!selector.validatedFrame(Protocol::Flex, t + 4s), "active POCSAG remains locked");
 require(!selector.validatedFrame(Protocol::Pocsag, t + 5s), "POCSAG refreshes lock");
 require(!selector.validatedFrame(Protocol::Flex, t + 18s), "new FLEX candidate begins");
 require(!selector.validatedFrame(Protocol::Flex, t + 19s), "two FLEX candidates insufficient");
 require(selector.validatedFrame(Protocol::Flex, t + 20s), "three FLEX frames switch after silence");
 require(selector.selected() == Protocol::Flex, "FLEX selected");
 require(!selector.validatedFrame(Protocol::Reflex, t + 34s), "ReFLEX first frame");
 require(!selector.validatedFrame(Protocol::Reflex, t + 35s), "ReFLEX second frame");
 require(selector.validatedFrame(Protocol::Reflex, t + 36s), "ReFLEX switch after validated frames");
 require(selector.selected() == Protocol::Reflex, "ReFLEX selected");
 require(!selector.validatedFrame(Protocol::Unknown, t + 37s), "unknown cannot change selection");
 selector.setMode(SelectionMode::Pocsag);
 require(selector.selected() == Protocol::Pocsag, "forced POCSAG");
 require(!selector.validatedFrame(Protocol::Flex, t + 100s), "forced mode ignores FLEX");
 selector.setMode(SelectionMode::Auto);
 require(selector.selected() == Protocol::Unknown, "return to AUTO resets evidence");
 require(!selector.validatedFrame(Protocol::Flex, t + 101s), "first FLEX candidate");
 require(!selector.validatedFrame(Protocol::Flex, t + 110s), "expired FLEX candidate starts over");
 require(selector.validatedFrame(Protocol::Flex, t + 111s), "second close FLEX frame confirms");
 selector.reset();
 require(selector.selected() == Protocol::Unknown, "retune clears AUTO lock");
 selector.setMode(SelectionMode::Reflex);
 require(selector.selected() == Protocol::Reflex, "forced ReFLEX");
 selector.setMode(SelectionMode::Flex);
 require(selector.selected() == Protocol::Flex, "forced FLEX");
 std::cout << "PASS: AUTO protocol hysteresis, POCSAG hold, FLEX/ReFLEX switching, timeouts, forced modes, retune reset\n";
}
