#include "dx12_command_reuse_supplement_test_support.hpp"
#include "dx12_srv_pool_test_support.hpp"
#include "../../src/renderer/dx12/dx12_synchronization_internal.hpp"
#include "../../src/renderer/dx12/dx12_recovery_internal.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include <array>

using namespace cgpui;
using namespace cgpui_test;
namespace {
struct HiddenRemovalCompletion final : Dx12SynchronizationApi {
  Dx12NativeSynchronizationApi native;
  unsigned signals = 0, reads = 0, events = 0, waits = 0;
  UINT64 ticket = 0; HRESULT signal_result = E_UNEXPECTED;
  HRESULT signal(ID3D12CommandQueue& q, ID3D12Fence& f, UINT64 value) noexcept override {
    ++signals; ticket = value; signal_result = native.signal(q, f, value); return signal_result;
  }
  UINT64 completed(ID3D12Fence&) noexcept override { ++reads; return 0; }
  HRESULT event(ID3D12Fence&, UINT64 value, HANDLE) noexcept override {
    ++events; return value == ticket ? E_ACCESSDENIED : E_INVALIDARG;
  }
  Dx12SynchronizationWaitResult wait(HANDLE) noexcept override { ++waits; return {WAIT_FAILED, ERROR_ACCESS_DENIED}; }
  HRESULT removed(ID3D12Device& device) noexcept override { return native.removed(device); }
  void pause() noexcept override { native.pause(); }
};
struct RestoreSynchronization { Dx12State& state; ~RestoreSynchronization() { state.synchronization_api = nullptr; } };
bool unknown_trace(const Dx12State& state, const ReuseQueueTail& tail, ID3D12CommandList* list) {
  unsigned producer = 0, verifier = 0, bundle = 0;
  for (const auto& event : state.command_observation->events()) if (event.parent == tail.parent) {
    if (event.kind == Dx12CommandEventKind::execute_bundle) ++bundle;
    if (event.kind == Dx12CommandEventKind::producer_execute) {
      if (verifier || event.object != state.queue.Get() || event.object2 != tail.target_identity) return false;
      ++producer;
    }
    if (event.kind == Dx12CommandEventKind::tail_execute) {
      if (producer != 1 || event.object != list || event.object2 != state.queue.Get()) return false;
      ++verifier;
    }
    if (event.kind == Dx12CommandEventKind::completed) return false;
  }
  return producer == 1 && verifier == 1 && bundle == 1 && !tail.failed && !tail.completed_parent && !tail.ticket;
}
}
int main() {
  Dx12RecoveryDeadline deadline; HiddenRemovalCompletion hidden; Dx12Window window;
  std::atomic_bool bundle_released = false, heap_released = false, tail_released = false;
  auto state = srv_native_state(&window);
  if (!deadline.done || !state) return dx12_fail(1, "unknown removal native setup");
  RestoreSynchronization restore{*state};
  state->command_observation = std::make_unique<Dx12CommandObservation>(16384);
  auto tail = std::make_unique<ReuseQueueTail>(); auto* verifier = tail.get();
  state->command_observation->verifier = std::move(tail);
  if (!reuse_scene(state, true, true) || !reuse_scene(state, false, false) ||
      !state->pixels || !reuse_trace_pixels_ready(*state, *verifier, state->pixels->rgba8))
    return dx12_fail(2, "unknown removal completed fresh prerequisite");
  auto* bundle = reuse_last_bundle(*state);
  if (!bundle || state->texture_renderer->native_textures.size() != 2 ||
      !reuse_native_sentinel(*bundle, bundle_released) ||
      !srv_heap_sentinel(*state->texture_renderer->native_textures.begin()->second.srv.heap(), heap_released))
    return dx12_fail(3, "unknown removal bundle/heap sentinels");
  const auto pixels = state->pixels->rgba8; const auto diagnostics = *state->diagnostics;
  const auto completed = reuse_event_count(*state, Dx12CommandEventKind::completed);
  const auto creates = reuse_event_count(*state, Dx12CommandEventKind::bundle_created);
  const auto parent = state->command_observation->parent_epoch();
  auto draws = reuse_raw_scene(*state);
  if (!draws) return dx12_fail(4, "unknown removal exact raw scene");
  state->synchronization_api = &hidden;
  const auto rejected = reuse_raw_submit(*state, *draws);
  const auto expected = dx12_error(E_ACCESSDENIED, "fence event registration");
  auto* pending_tail = verifier->prepared_list();
  if (rejected || rejected.error().code != expected.code || rejected.error().message != expected.message ||
      hidden.signals != 1 || FAILED(hidden.signal_result) || !hidden.ticket || hidden.reads != 1 ||
      hidden.events != 1 || hidden.waits || state->pending_fence_value != hidden.ticket || !state->submitted ||
      !pending_tail || !unknown_trace(*state, *verifier, pending_tail) || verifier->parent != parent + 1 ||
      reuse_event_count(*state, Dx12CommandEventKind::completed) != completed ||
      reuse_event_count(*state, Dx12CommandEventKind::bundle_created) != creates ||
      reuse_event_count(*state, Dx12CommandEventKind::bundle_released) ||
      reuse_event_count(*state, Dx12CommandEventKind::parent_released) ||
      bundle_released || heap_released || state->srv_pool->stats().live_slots != 2 ||
      state->pixels->rgba8 != pixels || !diagnostic_same(&*state->diagnostics, diagnostics) ||
      state->command_observation->overflowed() || !reuse_native_sentinel(*pending_tail, tail_released) ||
      !dx12_check_debug_errors(*state))
    return dx12_fail(5, "exact hidden-completion failure did not retain real producer/tail/bundle ownership");
  const auto before = state->info;
  // No fence wait or completion promotion: actual removal is the next retirement authority.
  if (!dx12_remove_native_device(before) || !state->submitted || state->pending_fence_value != hidden.ticket ||
      bundle_released || heap_released || tail_released)
    return dx12_fail(6, "actual removal unavailable or native owners retired before central release");
  state->synchronization_api = nullptr;
  { Dx12OperationGuard guard(*state);
    const auto recovered = dx12_recover_removed_device(*state);
    if (!recovered || !*recovered) return dx12_fail(7, "unknown removed state failed central native recovery");
  }
  if (!bundle_released || !heap_released || !tail_released || !reuse_parent_before_bundle_release(*state) ||
      reuse_event_count(*state, Dx12CommandEventKind::completed) != completed ||
      state->info.device_generation != before.device_generation + 1 || state->submitted ||
      state->pending_fence_value || state->srv_pool->stats().live_slots || verifier->completed_parent ||
      !verifier->failed || verifier->prepared_list() || state->command_observation->overflowed())
    return dx12_fail(8, "actual removal plus parent release failed unknown-work retirement without fake completion");
  state->command_observation = std::make_unique<Dx12CommandObservation>(16384);
  auto new_tail = std::make_unique<ReuseQueueTail>(); verifier = new_tail.get();
  state->command_observation->verifier = std::move(new_tail);
  if (!reuse_scene(state, false, true) || !state->pixels || state->pixels->rgba8 != pixels ||
      state->diagnostics->upload_bytes.image_byte_count != 20 || reuse_last_bundle(*state))
    return dx12_fail(9, "unknown removal new generation cold image initialization");
  std::array<std::uint64_t, 3> parents{};
  for (auto& epoch : parents) {
    if (!reuse_scene(state, false, false) || !reuse_trace_pixels_ready(*state, *verifier, pixels))
      return dx12_fail(10, "unknown removal new generation fresh/hit pixels");
    epoch = verifier->parent;
  }
  if (!reuse_native_history(state->command_observation->events(), parents) || !dx12_recovered_native_device_clean(state->info))
    return dx12_fail(11, "unknown removal new generation native reuse/debug");
  return 0;
}
