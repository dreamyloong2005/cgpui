#include "dx12_glyph_native_test_support.hpp"
#include <array>

using namespace cgpui;
using namespace cgpui_test;
int main() {
  std::array<Dx12TextureCopyEvent, 128> events{};
  Dx12TextureCopyObserver observer(events);
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "cold glyph fixture");
  state->texture_renderer->glyph_native->observer = &observer;
  glyph_add_record(*state, 1);
  auto a = glyph_publish(*state);
  glyph_add_record(*state, 2);
  auto ab = glyph_publish(*state);
  if (!a || !ab) return dx12_fail(2, "cold CPU snapshots");
  const auto p = a->pixels.size();
  std::array draws{glyph_draw(a, 0, 0), glyph_draw(ab, 20, 1)};
  if (!glyph_submit(*state, draws) || !glyph_pixels(*state, *a) || !glyph_pixels(*state, *ab, 20))
    return dx12_fail(3, "cold same-frame pixels/diagnostics");
  for (const auto& snapshot : {a, ab}) {
    auto page = glyph_readback(*state, *state->texture_renderer->native_textures.at(snapshot.get()).resource.Get());
    if (!page || *page != snapshot->pixels) return dx12_fail(4, "cold whole page changed");
  }
  if (glyph_copy_count(observer, Dx12TextureCopyKind::full_upload) != 2 ||
      glyph_copy_bytes(observer, Dx12TextureCopyKind::full_upload) != 2 * p ||
      glyph_copy_count(observer, Dx12TextureCopyKind::clone) || state->diagnostics->upload_bytes.glyph_atlas_byte_count != 2 * p)
    return dx12_fail(5, "pending A illegally seeded cold AB");
  // Prior completed AB: publish ABC and ABCD but traverse newer before older.
  glyph_add_record(*state, 3);
  auto abc = glyph_publish(*state);
  glyph_add_record(*state, 4);
  auto abcd = glyph_publish(*state);
  if (!abc || !abcd) return dx12_fail(6, "ordered successors");
  auto* base = state->texture_renderer->native_textures.at(ab.get()).resource.Get();
  draws = {glyph_draw(abcd, 0, 0), glyph_draw(abc, 20, 1)};
  observer.clear();
  if (!glyph_submit(*state, draws) || !glyph_pixels(*state, *abcd) || !glyph_pixels(*state, *abc, 20))
    return dx12_fail(7, "reverse successors native pixels");
  if (glyph_copy_count(observer, Dx12TextureCopyKind::clone) != 2 ||
      glyph_copy_bytes(observer, Dx12TextureCopyKind::region_upload) != 18 ||
      glyph_copy_count(observer, Dx12TextureCopyKind::region_upload) != 3)
    return dx12_fail(8, "same-frame successors must each copy suffix from prior completed AB");
  for (const auto& event : observer.events()) if (event.kind == Dx12TextureCopyKind::clone && event.source != base)
    return dx12_fail(9, "pending successor became another successor's base");
  glyph_add_record(*state, 5);
  auto abcde = glyph_publish(*state);
  auto next = glyph_draw(abcde);
  auto* highest = state->texture_renderer->native_textures.at(abcd.get()).resource.Get();
  observer.clear();
  if (!glyph_submit(*state, std::span{&next, 1}) || !glyph_pixels(*state, *abcde) ||
      glyph_copy_bytes(observer, Dx12TextureCopyKind::region_upload) != 6)
    return dx12_fail(10, "completion regressed catalog to last-traversed older prefix");
  for (const auto& event : observer.events()) if (event.kind == Dx12TextureCopyKind::clone && event.source != highest)
    return dx12_fail(11, "highest completed prefix not selected");
  observer.clear();
  next = glyph_draw(a);
  if (!glyph_submit(*state, std::span{&next, 1}) || !glyph_pixels(*state, *a) || !observer.events().empty())
    return dx12_fail(12, "exact old native snapshot must remain zero transport");
  return 0;
}
