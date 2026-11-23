#include "dx12_sync_retirement_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  int failures = 0;
  for (const bool fail_event : {true, false}) {
    Dx12ControlledRetirement script;
    script.event_result = fail_event ? E_ACCESSDENIED : S_OK;
    std::atomic_bool released = false;
    auto state = dx12_sync_state(script);
    if (!state) return dx12_sync_fail(1, "native state setup failed");
    if (!dx12_attach_resource_sentinel(*state, released) || released.load()) {
      state->submitted = false;
      return dx12_sync_fail(4, "resource sentinel attachment failed");
    }
    if (dx12_wait_idle(*state)) return dx12_sync_fail(2, "scripted event/wait failure was hidden");
    const bool retained = dx12_retirement_retains_until_complete(std::move(state), script, released);
    if (!retained || script.signal_calls != 1) {
      ++failures;
      dx12_sync_fail(3, fail_event
          ? "event failure must retire by polling its original ticket"
          : "wait failure must retire by polling its original ticket");
    }
  }
  return failures ? 3 : 0;
}
