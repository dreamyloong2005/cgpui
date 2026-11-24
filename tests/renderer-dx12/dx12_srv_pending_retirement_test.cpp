#include "dx12_srv_pool_test_support.hpp"
#include "dx12_sync_retirement_test_support.hpp"
#include "../../src/renderer/dx12/dx12_backdrop_blur_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  for (unsigned fault = 0; fault != 4; ++fault) {
    Dx12ControlledRetirement script;
    script.fail_signal_until_completion = fault == 0;
    script.event_result = fault == 1 ? E_ACCESSDENIED : S_OK;
    script.wait_result = fault == 3 ? Dx12SynchronizationWaitResult{WAIT_OBJECT_0, ERROR_SUCCESS}
                                   : Dx12SynchronizationWaitResult{WAIT_FAILED, ERROR_ACCESS_DENIED};
    std::atomic_bool released = false;
    auto state = std::make_unique<Dx12State>();
    state->width = state->height = 4;
    if (!dx12_create_device(*state)) return dx12_fail(1, "SRV retirement device unavailable");
    auto targets = dx12_create_backdrop_blur_targets(*state, {2, 2});
    if (!targets || !srv_heap_sentinel(*(*targets)->srvs.heap(), released))
      return dx12_fail(2, "SRV native heap release sentinel unavailable");
    state->backdrop_blur->frame_targets.push_back(std::move(*targets));
    state->synchronization_api = &script;
    state->submitted = true; // Synthetic pending only: never Execute native work.
    const auto result = dx12_wait_idle(*state);
    const bool retained = !result && state->submitted && !released.load() &&
        state->srv_pool->stats().page_count == 1 && state->srv_pool->stats().live_slots == 3;
    if (!retained) {
      state->submitted = false; // No native work exists in this controlled owner-state fixture.
      return dx12_fail(3, "Signal/event/wait/false-wake error freed unproven SRV owners");
    }
    if (!dx12_retirement_retains_until_complete(std::move(state), script, released))
      return dx12_fail(4, "state destructor freed descriptor heap before modeled completion or leaked after proof");
  }
  return 0;
}
