#include "dx12_command_reuse_transition_test_support.hpp"
#include <stdexcept>

namespace cgpui_test {
using namespace cgpui;
void transition_require(bool value,const std::string& why) {if(!value) throw std::runtime_error(why);}
TransitionFixture::TransitionFixture() : state(srv_native_state(&window)) {
  transition_require(bool(state),"transition real native state setup");
  const char* requested=std::getenv("CGPUI_DX12_ADAPTER");
  transition_require(!requested || std::string_view(requested)!="warp" || state->info.software_adapter,
      "requested WARP did not select software adapter");
  Dx12Ptr<ID3D12InfoQueue> debug;
  transition_require(!dx12_recovery_debug_requested() || SUCCEEDED(state->device.As(&debug)),"requested debug info queue unavailable");
  state->command_observation=std::make_unique<Dx12CommandObservation>(65536);
  auto owned=std::make_unique<ReuseQueueTail>();tail=owned.get();
  state->command_observation->verifier=std::move(owned);
}
void transition_pixels(const TransitionFrame& a,const TransitionFrame& b,bool same,const char* why) {
  transition_require((a.pixels==b.pixels)==same,why);
}
} // namespace cgpui_test
