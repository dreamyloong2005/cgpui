#include "dx12_sync_test_support.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;

namespace {
struct TerminalScript final : Dx12SyncScript {
  HRESULT removal = S_OK;
  HRESULT removed(ID3D12Device&) noexcept override { return removal; }
};
}

int main() {
  int failures = 0;
  for (const bool sentinel : {true, false}) {
    TerminalScript script;
    script.completion = sentinel ? std::numeric_limits<UINT64>::max() : 0;
    script.removal = sentinel ? S_OK : DXGI_ERROR_DEVICE_REMOVED;
    auto state = dx12_sync_state(script);
    if (!state) return dx12_sync_fail(1, "native state setup failed");
    const auto result = dx12_wait_idle(*state);
    if (result || result.error().message.find("887A0005") == std::string::npos || state->submitted) {
      ++failures;
      dx12_sync_fail(2, sentinel
          ? "removed fence sentinel with S_OK reason needs a terminal removal error"
          : "device removal must not be replaced by an ordinary synchronization error");
    }
    const auto signals = script.signal_calls;
    script.removal = S_OK;
    script.completion = 0;
    const auto repeated = dx12_wait_idle(*state);
    if (repeated || repeated.error().message.find("887A0005") == std::string::npos ||
        script.signal_calls != signals || state->submitted) {
      ++failures;
      dx12_sync_fail(4, "safe terminal retirement must not become a successful later wait");
    }
    state->submitted = false; // No native GPU submission occurred.
  }
  Dx12SyncScript script;
  auto state = dx12_sync_state(script);
  if (!state) return dx12_sync_fail(1, "native state setup failed");
  state->fence_value = std::numeric_limits<UINT64>::max() - 1;
  const auto exhausted = dx12_wait_idle(*state);
  if (exhausted || script.signal_calls != 0 || !state->submitted) {
    ++failures;
    dx12_sync_fail(3, "reserved removal sentinel must never be submitted as a fence ticket");
  }
  state->submitted = false;
  return failures ? 3 : 0;
}
