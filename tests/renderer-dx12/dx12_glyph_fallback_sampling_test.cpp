#include "dx12_glyph_native_test_support.hpp"
#include <array>

using namespace cgpui;
using namespace cgpui_test;
namespace {
Dx12DrawCommand scaled(Dx12TextureHandle snapshot) {
  auto result = glyph_draw(std::move(snapshot));
  auto& textured = std::get<Dx12TexturedDraw>(result.payload);
  textured.sampling = ImageSamplingMode::linear;
  for (auto& vertex : textured.vertices) {
    vertex.position[0] = vertex.position[0] * 2 + 0.25F;
    vertex.position[1] = vertex.position[1] * 2 + 0.25F;
  }
  return result;
}
}
int main() {
  std::array<Dx12TextureCopyEvent, 128> events{};
  Dx12TextureCopyObserver observer(events);
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "fallback sampling fixture");
  auto& renderer = *state->texture_renderer;
  renderer.glyph_native->observer = &observer;
  glyph_add_record(*state, 1);
  auto a = glyph_publish(*state);
  glyph_add_record(*state, 2);
  auto ab = glyph_publish(*state);
  if (!a || !ab) return dx12_fail(2, "fallback snapshots");
  auto draw = glyph_draw(ab);
  if (!glyph_submit(*state, std::span{&draw, 1}) || !glyph_pixels(*state, *ab))
    return dx12_fail(3, "newer cold base native pixels");
  auto* base = renderer.native_textures.at(ab.get()).resource.Get();
  observer.clear();
  draw = glyph_draw(a);
  if (!glyph_submit(*state, std::span{&draw, 1}) || !glyph_pixels(*state, *a) ||
      glyph_copy_count(observer, Dx12TextureCopyKind::full_upload) != 1 ||
      glyph_copy_bytes(observer, Dx12TextureCopyKind::full_upload) != a->pixels.size() ||
      glyph_copy_count(observer, Dx12TextureCopyKind::clone))
    return dx12_fail(4, "older uncached snapshot must fully initialize, never reverse-apply newer base");
  draw = scaled(a);
  if (!glyph_submit(*state, std::span{&draw, 1}) || !diagnostic_ink(&*state->pixels, 0, 0, 32, 32))
    return dx12_fail(5, "fractional scaled old snapshot blank");
  const auto before = state->pixels->rgba8;
  glyph_add_record(*state, 3);
  auto abc = glyph_publish(*state);
  observer.clear();
  draw = glyph_draw(abc);
  if (!glyph_submit(*state, std::span{&draw, 1}) || !glyph_pixels(*state, *abc) ||
      glyph_copy_bytes(observer, Dx12TextureCopyKind::region_upload) != 6)
    return dx12_fail(6, "older full fallback regressed completed-base prefix");
  for (const auto& event : observer.events()) if (event.kind == Dx12TextureCopyKind::clone && event.source != base)
    return dx12_fail(7, "successor cloned older fallback instead of prior maximum");
  auto old_page = glyph_readback(*state, *renderer.native_textures.at(a.get()).resource.Get());
  auto new_page = glyph_readback(*state, *renderer.native_textures.at(abc.get()).resource.Get());
  if (!old_page || *old_page != a->pixels || !new_page || *new_page != abc->pixels)
    return dx12_fail(8, "adjacent-edge source whole-page bytes changed");
  observer.clear(); draw = scaled(a);
  if (!glyph_submit(*state, std::span{&draw, 1}) || state->pixels->rgba8 != before || !observer.events().empty())
    return dx12_fail(9, "held old fractional/linear/2x sampling changed after neighboring glyph growth");
  return 0;
}
