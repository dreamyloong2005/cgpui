#include "dx12_command_reuse_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_synchronization_internal.hpp"
#include "../../src/renderer/dx12/dx12_native_retirement_internal.hpp"
#include <algorithm>

using namespace cgpui;
using namespace cgpui_test;
namespace {
struct HiddenCompletion final : Dx12SynchronizationApi {
  Dx12NativeSynchronizationApi native;
  HRESULT signal(ID3D12CommandQueue& q, ID3D12Fence& f, UINT64 value) noexcept override { return native.signal(q, f, value); }
  UINT64 completed(ID3D12Fence&) noexcept override { return 0; }
  HRESULT event(ID3D12Fence&, UINT64, HANDLE) noexcept override { return E_ACCESSDENIED; }
  Dx12SynchronizationWaitResult wait(HANDLE) noexcept override { return {WAIT_FAILED, ERROR_ACCESS_DENIED}; }
  HRESULT removed(ID3D12Device& device) noexcept override { return native.removed(device); }
  void pause() noexcept override { native.pause(); }
};
struct Restore { Dx12State& state; ~Restore() { state.synchronization_api = nullptr; } };
bool released(const Dx12State& state) {
  return std::ranges::any_of(state.command_observation->events(), [](const auto& e) {
    return e.kind == Dx12CommandEventKind::bundle_released;
  });
}
}
int main() {
  Dx12RecoveryDeadline deadline; HiddenCompletion hidden;
  Dx12Window window; auto state = srv_native_state(&window);
  if (!deadline.done || !state) return dx12_fail(1, "reuse unknown native setup");
  Restore restore{*state};
  state->command_observation = std::make_unique<Dx12CommandObservation>(8192);
  auto tail = std::make_unique<ReuseQueueTail>(); auto* verifier = tail.get();
  state->command_observation->verifier = std::move(tail);
  if (!reuse_scene(state, true, true) || !reuse_scene(state, false, false) || !dx12_check_debug_errors(*state))
    return dx12_fail(2, "reuse unknown fresh prerequisite");
  const auto pixels = state->pixels->rgba8;
  const auto diagnostics = *state->diagnostics;
  const auto previous_completion = verifier->completed_parent;
  state->synchronization_api = &hidden;
  if (reuse_scene(state, false, false) || !state->submitted || !state->pending_fence_value ||
      released(*state) || verifier->completed_parent == verifier->parent ||
      verifier->completed_parent != 0 || previous_completion >= verifier->parent ||
      state->pixels->rgba8 != pixels || !diagnostic_same(&*state->diagnostics, diagnostics))
    return dx12_fail(3, "notification failure published or released actual producer/tail work");
  state->synchronization_api = nullptr;
  if (!dx12_wait_idle(*state) || !reuse_trace_pixels_ready(*state, *verifier, pixels) || released(*state))
    return dx12_fail(4, "real ticket G failed or G alone prematurely retired parent bundle");
  // Public attempt failed: no recovery/retry/success is manufactured. Actual parent release supplies P.
  dx12_release_native_objects(*state);
  if (!released(*state) || state->device || state->srv_pool->stats().live_slots)
    return dx12_fail(5, "real G plus actual parent release did not retire old native owners");
  return 0;
}
