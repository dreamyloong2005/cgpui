#include "dx12_srv_pool_test_support.hpp"
#include "../../src/renderer/dx12/dx12_image_cache_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
std::unique_ptr<Dx12Frame> render(const std::shared_ptr<Dx12State>& state, bool green) {
  state->frame_active = true;
  auto frame = std::make_unique<Dx12Frame>(state);
  auto image = srv_tile_image(green ? 1 : 0);
  image.id = {9990};
  frame->clear({0, 0, 0, 1});
  frame->upload_image(image);
  frame->draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(image)});
  if (!frame->request_pixel_capture() || !frame->present() || !state->pixels ||
      !dx12_pixel(&*state->pixels, 8, 8, green ? std::array<unsigned char, 4>{0, 255, 0, 255}
                                             : std::array<unsigned char, 4>{255, 0, 0, 255}, 0)) return {};
  return frame; // Finished frame deliberately retains its immutable CPU draw.
}
bool empty(const std::shared_ptr<Dx12State>& state, std::size_t payload, bool invalidate = false) {
  state->frame_active = true;
  Dx12Frame frame(state);
  if (invalidate) frame.invalidate_image({9990});
  frame.clear({0, 0, 1, 1});
  return frame.request_pixel_capture() && frame.present() && state->pixels && state->diagnostics &&
      dx12_pixel(&*state->pixels, 8, 8, {0, 0, 255, 255}, 0) &&
      state->diagnostics->retained_image_byte_count == payload &&
      !state->diagnostics->retained_image_byte_count_saturated &&
      state->diagnostics->upload_bytes.image_byte_count == 0;
}
}
int main() {
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "SRV immutable history fixture unavailable");
  auto original = render(state, false);
  if (!original || state->texture_renderer->native_textures.size() != 1) return dx12_fail(2, "original red failed");
  const auto* old_source = state->texture_renderer->native_textures.begin()->first;
  const auto old_slot = state->texture_renderer->native_textures.begin()->second.srv.gpu().ptr;
  auto replacement = render(state, true);
  if (!replacement || state->texture_renderer->native_textures.size() != 2 ||
      !state->texture_renderer->native_textures.contains(old_source) || state->srv_pool->stats().live_slots != 2)
    return dx12_fail(3, "same-ID replacement recycled the historical frame's native realization");
  const Dx12Texture* new_source = nullptr;
  UINT64 new_slot = 0;
  for (const auto& [source, native] : state->texture_renderer->native_textures)
    if (source != old_source) { new_source = source; new_slot = native.srv.gpu().ptr; }
  if (!new_source || !new_slot || new_slot == old_slot)
    return dx12_fail(7, "historical snapshots must have distinct native slots");
  for (unsigned pass = 0; pass != 2; ++pass) {
    if (!empty(state, 8, pass == 0) || state->texture_renderer->native_textures.size() != 2 ||
        !state->texture_renderer->native_textures.contains(old_source) ||
        !state->texture_renderer->native_textures.contains(new_source) ||
        state->texture_renderer->native_textures.at(old_source).srv.gpu().ptr != old_slot ||
        state->texture_renderer->native_textures.at(new_source).srv.gpu().ptr != new_slot ||
        state->srv_pool->stats().live_slots != 2)
      return dx12_fail(8, "cache invalidation or empty-frame cleanup recycled live historical slots");
  }
  auto invalidated = srv_tile_image(0);
  invalidated.id = {9990};
  const auto absent = state->image_cache->lookup(describe_image_asset(invalidated));
  if (!absent || *absent) return dx12_fail(9, "history check did not actually invalidate current cache");
  auto guard = state->srv_pool->allocate(*state->device.Get(), 1);
  if (!guard || guard->gpu().ptr == old_slot) return dx12_fail(4, "historical descriptor was reused while its frame lived");
  *guard = {};
  original.reset();
  if (!empty(state, 4) || state->texture_renderer->native_textures.contains(old_source) ||
      !state->texture_renderer->native_textures.contains(new_source) ||
      state->texture_renderer->native_textures.at(new_source).srv.gpu().ptr != new_slot ||
      state->srv_pool->stats().live_slots != 1) return dx12_fail(5, "proven frame-start prune did not release only the old lease");
  auto recycled = state->srv_pool->allocate(*state->device.Get(), 1);
  if (!recycled || recycled->gpu().ptr != old_slot || state->srv_pool->stats().successful_page_creations != 1)
    return dx12_fail(6, "historical slot was not reusable after its native owner safely retired");
  *recycled = {};
  replacement.reset();
  if (!empty(state, 0) || !state->texture_renderer->native_textures.empty() || state->srv_pool->stats().live_slots)
    return dx12_fail(10, "final historical owner release did not retire the remaining native slot");
  return 0;
}
