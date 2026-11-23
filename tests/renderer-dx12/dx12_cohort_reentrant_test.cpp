#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_device_cohort_internal.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include "../../src/renderer/dx12/dx12_recovery_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
std::shared_ptr<Dx12State> participant(const Dx12Window& window) {
  auto state = std::make_shared<Dx12State>();
  Dx12OperationGuard gate(*state);
  state->hwnd = window.hwnd;
  state->width = state->height = 64;
  state->scale = {1};
  if (!dx12_create_device(*state) || !dx12_create_pipeline(*state) ||
      !dx12_create_swapchain(*state) || !dx12_check_debug_errors(*state)) return {};
  dx12_register_device_cohort(state);
  return state;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done) return dx12_fail(1, "reentrant-cohort watchdog unavailable");
  Dx12Window first_window, second_window;
  if (!first_window.hwnd || !second_window.hwnd) return dx12_fail(2, "reentrant-cohort windows unavailable");
  auto origin = participant(first_window), peer = participant(second_window);
  if (!origin || !peer) return dx12_fail(3, "reentrant-cohort native initialization failed");
  if (origin->info.adapter_luid != peer->info.adapter_luid ||
      origin->window_thread_id != GetCurrentThreadId() ||
      peer->window_thread_id != GetCurrentThreadId()) {
    return dx12_fail(4, "reentrant-cohort participants do not share adapter and HWND owner");
  }
  auto* const device = peer->device.Get();
  auto* const commands = peer->commands.Get();
  if (!dx12_remove_native_device(origin->info)) return dx12_fail(5, "reentrant-cohort removal unverified");

  Dx12OperationGuard origin_gate(*origin);
  {
    // Private seam: model a still-active peer entry on this same thread. This
    // is not a claim that the test injected an actual DXGI window callback.
    Dx12OperationGuard active_peer_entry(*peer);
    dx12_retire_removed_cohort_peers(*origin);
    if (peer->recovery_pending || peer->failed || peer->device.Get() != device ||
        peer->commands.Get() != commands) {
      return dx12_fail(6, "same-thread recursive lock was mistaken for an idle peer operation");
    }
  }
  dx12_retire_removed_cohort_peers(*origin);
  if (!peer->recovery_pending || peer->device || peer->commands) {
    return dx12_fail(7, "completed peer operation did not become eligible for retirement");
  }
  {
    Dx12OperationGuard outer_peer(*peer);
    Dx12OperationGuard nested_peer(*peer);
    const auto nested = dx12_recover_removed_device(*peer);
    if (nested || nested.error().code == ErrorCode::frame_retry_required ||
        !peer->recovery_pending || peer->device)
      return dx12_fail(10, "nested pending peer rebuilt inside an active outer operation");
  }
  auto* const original_device = origin->device.Get();
  const auto original_generation = origin->info.device_generation;
  {
    Dx12OperationGuard nested_origin_entry(*origin);
    const auto nested = dx12_recover_removed_device(*origin);
    if (nested || nested.error().code == ErrorCode::frame_retry_required ||
        origin->device.Get() != original_device || origin->recovery_pending ||
        origin->info.device_generation != original_generation) {
      return dx12_fail(8, "nested recovery retired its own active outer operation");
    }
  }
  const auto recovered = dx12_recover_removed_device(*origin);
  if (!recovered || !*recovered || origin->info.device_generation != original_generation + 1)
    return dx12_fail(9, "outer safe boundary did not recover after nested rejection");
  return 0;
}
