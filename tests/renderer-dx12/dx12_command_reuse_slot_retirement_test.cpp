#include "dx12_command_reuse_supplement_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_native_internal.hpp"
#include "../../src/renderer/dx12/dx12_image_cache_internal.hpp"
#include <array>
#include <algorithm>

int main() {
  using namespace cgpui; using namespace cgpui_test;
  Dx12RecoveryDeadline deadline; ReuseFailListReset failure;
  Dx12Window window; auto state = srv_native_state(&window);
  if (!deadline.done || !state) return dx12_fail(1, "reuse slot native setup");
  state->srv_pool = std::make_unique<Dx12SrvDescriptorPool>(3); // Existing supported minimum; not a production option.
  state->command_observation = std::make_unique<Dx12CommandObservation>(16384);
  if (!reuse_scene(state, true, true) || !reuse_scene(state, false, false) || !reuse_last_bundle(*state))
    return dx12_fail(2, "reuse slot fresh bundle prerequisite");
  std::array<UINT64, 2> slots{}; std::array<std::weak_ptr<const Dx12Texture>, 2> weak;
  ID3D12DescriptorHeap* original = nullptr; unsigned count = 0;
  for (const auto& [key, native] : state->texture_renderer->native_textures) {
    if (count >= 2 || (original && original != native.srv.heap())) return dx12_fail(3, "unexpected image topology");
    original = native.srv.heap(); slots[count] = native.srv.gpu().ptr; weak[count++] = native.source;
  }
  if (count != 2) return dx12_fail(4, "missing immutable native image nodes");
  auto filler = state->srv_pool->allocate(*state->device.Get(), 1);
  if (!filler || filler->heap() != original) return dx12_fail(5, "third slot fill");
  state->image_cache->invalidate({9900}); state->image_cache->invalidate({9901});
  const auto resets = reuse_event_count(*state, Dx12CommandEventKind::parent_reset);
  const auto executes = reuse_event_count(*state, Dx12CommandEventKind::producer_execute);
  const auto parent = state->command_observation->parent_epoch();
  state->texture_renderer->glyph_native->command_api = &failure;
  const auto denied = reuse_raw_submit(*state, {}); // Actual allocator Reset; denied list Reset is NOT P.
  state->texture_renderer->glyph_native->command_api = nullptr;
  const auto expected = dx12_error(E_ACCESSDENIED, "command list reset");
  if (denied || denied.error().code != expected.code || denied.error().message != expected.message ||
      failure.allocator_calls != 1 || FAILED(failure.allocator_result) || failure.list_calls != 1 ||
      failure.list_result != E_ACCESSDENIED || failure.close_calls != 0 || state->submitted ||
      state->command_observation->overflowed() || state->command_observation->parent_epoch() != parent ||
      reuse_event_count(*state, Dx12CommandEventKind::parent_reset) != resets ||
      reuse_event_count(*state, Dx12CommandEventKind::producer_execute) != executes)
    return dx12_fail(6, "target Reset denial missing, allocator failed, or parent/submission advanced");
  if (weak[0].expired() || weak[1].expired() ||
      state->srv_pool->stats().live_slots != 3 || reuse_event_count(*state, Dx12CommandEventKind::bundle_released))
    return dx12_fail(7, "G-only/failed Reset released entry CPU pins or sole SRV slots");
  auto extra = state->srv_pool->allocate(*state->device.Get(), 1);
  if (!extra || extra->heap() == original || extra->gpu().ptr == slots[0] || extra->gpu().ptr == slots[1])
    return dx12_fail(8, "live bundle slots recycled while old parent still references entry");
  if (!reuse_raw_submit(*state, {}) || !weak[0].expired() || !weak[1].expired() ||
      reuse_event_count(*state, Dx12CommandEventKind::bundle_released) != 1 || state->srv_pool->stats().live_slots != 4)
    return dx12_fail(9, "successful P failed to drop entry pins or pruned ahead of accepted ordering");
  if (!reuse_raw_submit(*state, {}) || state->srv_pool->stats().live_slots != 2)
    return dx12_fail(10, "next ordinary prune failed to return both retired slots");
  auto first = state->srv_pool->allocate(*state->device.Get(), 1), second = state->srv_pool->allocate(*state->device.Get(), 1);
  std::sort(slots.begin(), slots.end());
  if (!first || !second || first->heap() != original || second->heap() != original)
    return dx12_fail(11, "safe retirement did not make original page reusable");
  std::array<UINT64,2> reused{first->gpu().ptr, second->gpu().ptr}; std::sort(reused.begin(), reused.end());
  if (reused != slots || !dx12_check_debug_errors(*state)) return dx12_fail(12, "retired exact slots not recycled/debug failed");
  // Local unreferenced leases destruct before state/pool; no central clear with outstanding fixture allocations.
  return 0;
}
