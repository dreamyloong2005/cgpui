#include "dx12_command_reuse_supplement_test_support.hpp"
#include "dx12_srv_pool_test_support.hpp"
#include "../../src/renderer/dx12/dx12_recovery_internal.hpp"
#include "../../src/renderer/dx12/dx12_device_cohort_internal.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"

int main() {
  using namespace cgpui; using namespace cgpui_test;
  Dx12RecoveryDeadline deadline; Dx12Window first_window, peer_window;
  // Sentinel storage outlives both states even on an early failure return.
  std::atomic_bool first_bundle_released = false, peer_bundle_released = false;
  std::atomic_bool first_heap_released = false, peer_heap_released = false;
  auto first = srv_native_state(&first_window), peer = srv_native_state(&peer_window);
  if (!deadline.done || !first || !peer || first->info.adapter_luid != peer->info.adapter_luid)
    return dx12_fail(1, "reuse cohort native same-adapter setup");
  first->command_observation = std::make_unique<Dx12CommandObservation>(16384);
  peer->command_observation = std::make_unique<Dx12CommandObservation>(16384);
  if (!reuse_scene(first, true, true) || !reuse_scene(first, false, false) ||
      !reuse_scene(peer, true, true) || !reuse_scene(peer, false, false))
    return dx12_fail(2, "two real fresh bundle prerequisites");
  auto* a = reuse_last_bundle(*first); auto* b = reuse_last_bundle(*peer);
  auto* ah = first->texture_renderer->native_textures.begin()->second.srv.heap();
  auto* bh = peer->texture_renderer->native_textures.begin()->second.srv.heap();
  if (!a || !b || a == b || ah == bh ||
      !reuse_native_sentinel(*a, first_bundle_released) || !reuse_native_sentinel(*b, peer_bundle_released) ||
      !srv_heap_sentinel(*ah, first_heap_released) || !srv_heap_sentinel(*bh, peer_heap_released))
    return dx12_fail(3, "bundle/list and per-state heap native sentinels unavailable");
  { Dx12OperationGuard guard(*first); dx12_register_device_cohort(first); }
  { Dx12OperationGuard guard(*peer); dx12_register_device_cohort(peer); }
  peer->frame_active = true;
  auto orphan = std::make_unique<Dx12Frame>(peer);
  orphan->draw_image({.bounds = {{0,0},{16,16}}, .asset = describe_image_asset(srv_tile_image(0))});
  peer->owner_alive = false; // Existing private ownerless-state seam, not simulated device completion.
  const auto before = first->info; const auto peer_generation = peer->info.device_generation;
  if (!dx12_remove_native_device(before)) return dx12_fail(4, "actual RemoveDevice unavailable");
  { Dx12OperationGuard guard(*first);
    const auto recovered = dx12_recover_removed_device(*first);
    if (!recovered || !*recovered) return dx12_fail(5, "origin rebuild blocked by old cohort bundle consumers");
  }
  if (!first_bundle_released || !peer_bundle_released || !first_heap_released || !peer_heap_released ||
      !reuse_parent_before_bundle_release(*first) || !reuse_parent_before_bundle_release(*peer) ||
      peer->device || !peer->recovery_pending || peer->info.device_generation != peer_generation ||
      first->info.device_generation != before.device_generation + 1 ||
      first->srv_pool->stats().live_slots || peer->srv_pool->stats().live_slots)
    return dx12_fail(6, "actual removed G/parent P failed native bundle/heap retirement or rebuilt orphan");
  const auto rejected = orphan->present();
  if (rejected || rejected.error().code != ErrorCode::invalid_argument || peer->device ||
      peer->info.device_generation != peer_generation) return dx12_fail(7, "ownerless peer rebuilt or returned retry");
  // Old attachment has already retired all resources. Replace it for new-epoch pixel evidence.
  first->command_observation = std::make_unique<Dx12CommandObservation>(16384);
  auto tail = std::make_unique<ReuseQueueTail>(); auto* verifier = tail.get();
  first->command_observation->verifier = std::move(tail);
  if (!reuse_scene(first, false, true) || !first->pixels ||
      first->diagnostics->upload_bytes.image_byte_count != 20 || reuse_last_bundle(*first))
    return dx12_fail(8, "recovered cold DIRECT did not initialize retained CPU images exactly");
  const auto expected = first->pixels->rgba8;
  std::array<std::uint64_t, 3> parents{};
  for (auto& parent : parents) {
    if (!reuse_scene(first, false, false) || !reuse_trace_pixels_ready(*first, *verifier, expected))
      return dx12_fail(9, "new generation fresh/hit queue-tail pixels failed");
    parent = verifier->parent;
  }
  if (!reuse_native_history(first->command_observation->events(), parents) || !dx12_recovered_native_device_clean(first->info))
    return dx12_fail(10, "new epoch did not close fresh once then reuse actual native recording cleanly");
  return 0;
}
