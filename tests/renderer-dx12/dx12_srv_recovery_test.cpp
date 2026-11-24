#include "dx12_srv_pool_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_recovery_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_recovery_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  std::atomic_bool released = false;
  auto state = srv_native_state(&window);
  if (!deadline.done || !state || !srv_render_tiles(state, true, true) || !srv_topology(*state, 14, 1))
    return dx12_fail(1, "SRV recovery native scene unavailable");
  state->frame_active = true;
  auto held = std::make_unique<Dx12Frame>(state);
  held->draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(srv_tile_image(0))});
  if (!held->present()) return dx12_fail(2, "retained historical CPU frame unavailable");
  auto* heap = state->texture_renderer->native_textures.begin()->second.srv.heap();
  if (!srv_heap_sentinel(*heap, released)) return dx12_fail(3, "recovery heap sentinel unavailable");
  const auto before = state->info;
  const auto creates = state->srv_pool->stats().successful_page_creations;
  for (const float width : {79.0F, 64.0F}) {
    if (!dx12_resize_recovering(*state, {width, 64}, {1}) || !srv_render_tiles(state, false, true) ||
        released.load() || state->srv_pool->stats().successful_page_creations != creates)
      return dx12_fail(4, "same-device odd resize replaced the SRV pool or changed pixels");
  }
  if (!dx12_remove_native_device(before)) return dx12_fail(5, "real device removal failed");
  const auto recovered = dx12_recover_removed_device(*state);
  if (!recovered || !*recovered || !released.load() || state->srv_pool->stats().page_count ||
      state->srv_pool->stats().live_slots || state->srv_pool->stats().successful_page_creations != creates ||
      state->info.device_generation != before.device_generation + 1)
    return dx12_fail(6, "held CPU snapshots retained removed-epoch SRV heaps");
  if (!srv_render_tiles(state, false, true, 32)) {
    // CPU caches survive; only native resources upload again, without new upload_image calls.
    return dx12_fail(7, "recovered descriptors did not reupload exact image pixels");
  }
  if (state->srv_pool->stats().successful_page_creations != creates + 1 ||
      !srv_topology(*state, 14, 1) || !srv_render_tiles(state, false, true) ||
      state->srv_pool->stats().successful_page_creations != creates + 1 ||
      !dx12_recovered_native_device_clean(state->info))
    return dx12_fail(8, "new epoch did not create one page then reuse it cleanly");
  return 0;
}
