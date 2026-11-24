#include "dx12_srv_pool_test_support.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
std::unique_ptr<Dx12Frame> render(const std::shared_ptr<Dx12State>& state, const char* glyph) {
  state->frame_active = true;
  auto frame = std::make_unique<Dx12Frame>(state);
  frame->clear({0, 0, 0, 1});
  frame->draw_text(diagnostic_text(glyph, 16, {0, 0}));
  if (!frame->request_pixel_capture() || !frame->present() || !state->pixels ||
      !diagnostic_ink(&*state->pixels, 0, 0, 24, 24)) return {};
  return frame;
}
}
int main() {
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "SRV glyph history fixture failed");
  auto original = render(state, "M");
  if (!original || state->texture_renderer->native_textures.size() != 1) return dx12_fail(2, "first glyph snapshot failed");
  const auto* old_source = state->texture_renderer->native_textures.begin()->first;
  const auto old_slot = state->texture_renderer->native_textures.begin()->second.srv.gpu().ptr;
  auto newer = render(state, "W");
  if (!newer || state->texture_renderer->native_textures.size() != 2 ||
      !state->texture_renderer->native_textures.contains(old_source) || state->srv_pool->stats().live_slots != 2 ||
      state->texture_renderer->native_textures.at(old_source).srv.gpu().ptr != old_slot)
    return dx12_fail(3, "glyph atlas republish overwrote the historical native descriptor");
  original.reset();
  state->frame_active = true;
  Dx12Frame idle(state);
  if (!idle.present() || state->texture_renderer->native_textures.contains(old_source) || state->srv_pool->stats().live_slots != 1)
    return dx12_fail(4, "old glyph descriptor was not released at the first proven prune");
  auto reused = state->srv_pool->allocate(*state->device.Get(), 1);
  return reused && reused->gpu().ptr == old_slot ? 0 : dx12_fail(5, "retired glyph slot was not reusable");
}
