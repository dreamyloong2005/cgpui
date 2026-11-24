#include "dx12_command_reuse_supplement_test_support.hpp"
#include "dx12_srv_pool_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_native_internal.hpp"
#include "../../src/renderer/dx12/dx12_native_retirement_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
struct CloseDenial final : Dx12GlyphCommandApi {
  unsigned allocators = 0, lists = 0, closes = 0;
  HRESULT allocator_result = E_UNEXPECTED, list_result = E_UNEXPECTED, close_result = E_UNEXPECTED;
  HRESULT reset_allocator(ID3D12CommandAllocator& allocator) noexcept override {
    ++allocators; allocator_result = dx12_native_glyph_command_api().reset_allocator(allocator); return allocator_result;
  }
  HRESULT reset_list(ID3D12GraphicsCommandList& list, ID3D12CommandAllocator& allocator, ID3D12PipelineState* pso) noexcept override {
    ++lists; list_result = dx12_native_glyph_command_api().reset_list(list, allocator, pso); return list_result;
  }
  HRESULT close(ID3D12GraphicsCommandList&) noexcept override { ++closes; close_result = E_ACCESSDENIED; return close_result; }
};
struct RestoreCommands { Dx12State& state; ~RestoreCommands() { state.texture_renderer->glyph_native->command_api = nullptr; } };
}
int main() {
  Dx12RecoveryDeadline deadline; CloseDenial denied_close; Dx12Window window;
  std::atomic_bool bundle_released = false, heap_released = false;
  auto state = srv_native_state(&window);
  if (!deadline.done || !state) return dx12_fail(1, "exact Close native setup");
  RestoreCommands restore{*state};
  state->command_observation = std::make_unique<Dx12CommandObservation>(16384);
  auto tail = std::make_unique<ReuseQueueTail>(); auto* verifier = tail.get();
  state->command_observation->verifier = std::move(tail);
  if (!reuse_scene(state, true, true) || !reuse_scene(state, false, false) || !state->pixels ||
      !reuse_trace_pixels_ready(*state, *verifier, state->pixels->rgba8))
    return dx12_fail(2, "exact Close real completed fresh prerequisite");
  auto* bundle = reuse_last_bundle(*state);
  if (!bundle || state->texture_renderer->native_textures.size() != 2 ||
      !reuse_native_sentinel(*bundle, bundle_released) ||
      !srv_heap_sentinel(*state->texture_renderer->native_textures.begin()->second.srv.heap(), heap_released))
    return dx12_fail(3, "exact Close native sentinels");
  const auto pixels = state->pixels->rgba8; const auto diagnostics = *state->diagnostics;
  const auto submissions = reuse_event_count(*state, Dx12CommandEventKind::producer_execute);
  const auto tails = reuse_event_count(*state, Dx12CommandEventKind::tail_execute);
  const auto resets = reuse_event_count(*state, Dx12CommandEventKind::parent_reset);
  const auto executions = reuse_event_count(*state, Dx12CommandEventKind::execute_bundle);
  const auto creates = reuse_event_count(*state, Dx12CommandEventKind::bundle_created);
  const auto parent = state->command_observation->parent_epoch();
  auto draws = reuse_raw_scene(*state);
  if (!draws) return dx12_fail(4, "exact Close raw scene");
  state->texture_renderer->glyph_native->command_api = &denied_close;
  const auto rejected = reuse_raw_submit(*state, *draws);
  state->texture_renderer->glyph_native->command_api = nullptr;
  const auto expected = dx12_error(E_ACCESSDENIED, "command list close");
  if (rejected || rejected.error().code != expected.code || rejected.error().message != expected.message ||
      denied_close.allocators != 1 || FAILED(denied_close.allocator_result) || denied_close.lists != 1 ||
      FAILED(denied_close.list_result) || denied_close.closes != 1 || denied_close.close_result != E_ACCESSDENIED ||
      state->submitted || state->pending_fence_value || state->command_observation->parent_epoch() != parent + 1 ||
      reuse_event_count(*state, Dx12CommandEventKind::parent_reset) != resets + 1 ||
      reuse_event_count(*state, Dx12CommandEventKind::execute_bundle) != executions + 1 ||
      reuse_event_count(*state, Dx12CommandEventKind::bundle_created) != creates ||
      reuse_event_count(*state, Dx12CommandEventKind::producer_execute) != submissions ||
      reuse_event_count(*state, Dx12CommandEventKind::tail_execute) != tails ||
      reuse_event_count(*state, Dx12CommandEventKind::bundle_released) || bundle_released || heap_released ||
      state->srv_pool->stats().live_slots != 2 || state->pixels->rgba8 != pixels ||
      !diagnostic_same(&*state->diagnostics, diagnostics) || state->command_observation->overflowed())
    return dx12_fail(5, "exact Close denial missing or unsubmitted referenced BUNDLE was executed/published/released");
  unsigned close_events = 0;
  for (const auto& event : state->command_observation->events())
    if (event.parent == parent + 1 && event.kind == Dx12CommandEventKind::parent_closed) {
      if (event.result != E_ACCESSDENIED) return dx12_fail(6, "actual Close observer lost target HRESULT");
      ++close_events;
    }
  if (close_events != 1 || !dx12_check_debug_errors(*state)) return dx12_fail(7, "exact Close provenance/debug");
  // Prior native work completed; this newly referenced parent NEVER executed. Actual release supplies P.
  dx12_release_native_objects(*state);
  if (!bundle_released || !heap_released || !reuse_parent_before_bundle_release(*state) ||
      state->srv_pool->stats().live_slots || state->device || state->command_observation->overflowed())
    return dx12_fail(8, "actual parent release failed to retire unsubmitted BUNDLE/sole slots");
  return 0;
}
