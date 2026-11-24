#include "dx12_srv_pool_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_recovery_internal.hpp"
#include "../../src/renderer/dx12/dx12_device_cohort_internal.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window first_window, peer_window;
  std::atomic_bool first_released = false, peer_released = false;
  auto first = srv_native_state(&first_window), peer = srv_native_state(&peer_window);
  if (!deadline.done || !first || !peer || first->info.adapter_luid != peer->info.adapter_luid ||
      !srv_render_tiles(first, true, false) || !srv_render_tiles(peer, true, false))
    return dx12_fail(1, "SRV same-adapter cohort fixture failed");
  { Dx12OperationGuard lock(*first); dx12_register_device_cohort(first); }
  { Dx12OperationGuard lock(*peer); dx12_register_device_cohort(peer); }
  auto* a = first->texture_renderer->native_textures.begin()->second.srv.heap();
  auto* b = peer->texture_renderer->native_textures.begin()->second.srv.heap();
  if (a == b || !srv_heap_sentinel(*a, first_released) || !srv_heap_sentinel(*b, peer_released))
    return dx12_fail(2, "per-state pages not distinct or native sentinels unavailable");
  peer->frame_active = true;
  auto orphan = std::make_unique<Dx12Frame>(peer);
  orphan->draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(srv_tile_image(1))});
  peer->owner_alive = false; // Private state seam; public facade deletion remains covered by orphan regressions.
  const auto peer_generation = peer->info.device_generation;
  const auto before = first->info;
  if (!dx12_remove_native_device(before)) return dx12_fail(3, "SRV real cohort removal unavailable");
  { Dx12OperationGuard lock(*first);
    const auto recovered = dx12_recover_removed_device(*first);
    if (!recovered || !*recovered) return dx12_fail(4, "origin recovery was blocked by orphan native SRV owners");
  }
  if (!first_released.load() || !peer_released.load() || first->srv_pool->stats().page_count ||
      peer->srv_pool->stats().page_count || peer->device || !peer->recovery_pending ||
      peer->info.device_generation != peer_generation || first->info.device_generation != before.device_generation + 1)
    return dx12_fail(5, "cohort purge did not release both old pages or rebuilt an ownerless peer");
  const auto failed = orphan->present();
  if (failed || failed.error().code != ErrorCode::invalid_argument || peer->device ||
      peer->srv_pool->stats().successful_page_creations != 1 ||
      !srv_render_tiles(first, false, false, 32) || !srv_render_tiles(first, false, false))
    return dx12_fail(6, "ownerless removal reconstructed or live peer failed cold/warm reupload");
  return dx12_recovered_native_device_clean(first->info) ? 0 : dx12_fail(7, "cohort new-epoch validation error");
}
