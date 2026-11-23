#include "dx12_retirement_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_synchronization_internal.hpp"

namespace cgpui {
void dx12_retire_submitted_work(Dx12State& state) noexcept {
  if (!state.submitted) return;
  Dx12NativeSynchronizationApi native;
  auto& api = state.synchronization_api ? *state.synchronization_api : native;
  while (dx12_poll_submission(state, api).progress == Dx12SynchronizationProgress::pending) {
    // Event registration and event waits are optional notification paths.
    // Retain every state-owned resource until fence/removal evidence exists.
    api.pause();
  }
}
} // namespace cgpui
