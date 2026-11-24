#include "dx12_glyph_native_test_support.hpp"
#include <array>

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "observer capacity fixture");
  glyph_add_record(*state, 1);
  auto a = glyph_publish(*state);
  auto draw = glyph_draw(a);
  if (!glyph_submit(*state, std::span{&draw, 1}) || !glyph_pixels(*state, *a))
    return dx12_fail(2, "observer completed A");
  const auto pixels = state->pixels->rgba8;
  const auto diagnostics = *state->diagnostics;
  glyph_add_record(*state, 2); glyph_add_record(*state, 3);
  auto ab = glyph_publish(*state);
  std::array<Dx12TextureCopyEvent, 1> storage{};
  Dx12TextureCopyObserver observer(storage);
  auto& owner = *state->texture_renderer->glyph_native;
  owner.observer = &observer;
  draw = glyph_draw(ab);
  const auto result = dx12_submit_frame(*state, {0, 0, 0, 1}, std::span{&draw, 1}, true, 1);
  owner.observer = nullptr;
  if (result || result.error().code != ErrorCode::invalid_argument || !observer.events().empty() ||
      observer.overflowed() || state->submitted || state->pixels->rgba8 != pixels ||
      !diagnostic_same(&*state->diagnostics, diagnostics) || owner.stats().close_attempts != 1 ||
      FAILED(owner.stats().last_close) || owner.stats().candidates ||
      state->texture_renderer->native_textures.at(ab.get()).resource)
    return dx12_fail(3, "bounded observer failure occurred after target references or published success");
  return 0;
}
