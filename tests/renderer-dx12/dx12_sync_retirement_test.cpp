#include "dx12_sync_retirement_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12ControlledRetirement script;
  script.fail_signal_until_completion = true;
  std::atomic_bool released = false;
  auto state = dx12_sync_state(script);
  if (!state) return dx12_sync_fail(1, "native state setup failed");
  if (!dx12_attach_resource_sentinel(*state, released) || released.load()) {
    state->submitted = false;
    return dx12_sync_fail(6, "resource sentinel attachment failed");
  }
  if (dx12_wait_idle(*state)) return dx12_sync_fail(2, "scripted Signal failure was hidden");
  if (!dx12_retirement_retains_until_complete(std::move(state), script, released)) {
    return dx12_sync_fail(4, "destructor released state after repeated unproven Signal failure");
  }
  if (script.signal_calls < 3) return dx12_sync_fail(5, "retirement did not retry failed Signal");
  return 0;
}
