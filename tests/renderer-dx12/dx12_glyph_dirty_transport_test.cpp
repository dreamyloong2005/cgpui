#include "dx12_glyph_native_test_support.hpp"
#include <array>
#include <iostream>
#include <string_view>

using namespace cgpui;
using namespace cgpui_test;
namespace {
int run(bool hold_a) {
  std::array<Dx12TextureCopyEvent, 128> events{};
  Dx12TextureCopyObserver observer(events);
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "glyph native fixture unavailable");
  state->texture_renderer->glyph_native->observer = &observer;
  glyph_add_record(*state, 1);
  auto a = glyph_publish(*state);
  if (!a) return dx12_fail(2, "glyph A publication");
  auto draw_a = glyph_draw(a);
  if (!glyph_submit(*state, std::span{&draw_a, 1}) || !glyph_pixels(*state, *a))
    return dx12_fail(3, "glyph A native pixels/diagnostics");
  const auto p = a->pixels.size();
  auto* old_native = state->texture_renderer->native_textures.at(a.get()).resource.Get();
  const auto old_bytes = a->pixels;
  auto old_read = glyph_readback(*state, *old_native);
  if (!old_read || *old_read != old_bytes || glyph_copy_bytes(observer, Dx12TextureCopyKind::full_upload) != p)
    return dx12_fail(4, "glyph A full readback/upload");
  observer.clear();
  if (!glyph_submit(*state, std::span{&draw_a, 1}) || !glyph_pixels(*state, *a) || !observer.events().empty())
    return dx12_fail(5, "cached A must have zero native transport");
  std::weak_ptr<const Dx12Texture> weak_a = a;
  glyph_add_record(*state, 2); //3x2, padded stride5.
  glyph_add_record(*state, 3, 5, 3, 8);
  auto ab = glyph_publish(*state);
  if (!ab) return dx12_fail(6, "glyph AB publication");
  draw_a = {}; // Drop the old draw's CPU handle in the released case.
  if (!hold_a) a.reset();
  auto draw_ab = glyph_draw(ab);
  observer.clear();
  if (!glyph_submit(*state, std::span{&draw_ab, 1}) || !glyph_pixels(*state, *ab))
    return dx12_fail(7, "glyph AB native pixels/diagnostics");
  auto* target = state->texture_renderer->native_textures.at(ab.get()).resource.Get();
  auto read = glyph_readback(*state, *target);
  if (!read || *read != ab->pixels || (hold_a ? weak_a.expired() : !weak_a.expired()))
    return dx12_fail(8, "glyph AB full readback/distinct COW/CPU ownership");
  if (hold_a) {
    auto unchanged = glyph_readback(*state, *old_native);
    if (a->pixels != old_bytes || !unchanged || *unchanged != old_bytes)
      return dx12_fail(9, "held A changed after successor upload");
  }
  std::cout << "glyph exact native pixels/full pages and planned/submitted work passed "
            << (hold_a ? "held" : "released") << '\n';
  // Dedicated semantic transport oracle, reached only after the pixel/full-page checks.
  if (target == old_native || glyph_copy_count(observer, Dx12TextureCopyKind::allocation) != 1 ||
      glyph_copy_count(observer, Dx12TextureCopyKind::clone) != 1 ||
      glyph_copy_bytes(observer, Dx12TextureCopyKind::clone) != p ||
      glyph_copy_count(observer, Dx12TextureCopyKind::region_upload) != 2 ||
      glyph_copy_bytes(observer, Dx12TextureCopyKind::region_upload) != 21 ||
      glyph_copy_count(observer, Dx12TextureCopyKind::full_upload) != 0 ||
      state->diagnostics->upload_bytes.glyph_atlas_byte_count != 21 || observer.overflowed())
    return dx12_fail(40, "glyph eligible prior-completed transport must clone P and upload exact two-region D");
  bool source_seen = false;
  for (const auto& event : observer.events()) if (event.kind == Dx12TextureCopyKind::clone)
    source_seen = event.source == old_native && event.target == target;
  if (!source_seen) return dx12_fail(41, "clone did not use the prior completed resource");
  observer.clear();
  if (!glyph_submit(*state, std::span{&draw_ab, 1}) || !glyph_pixels(*state, *ab) || !observer.events().empty())
    return dx12_fail(42, "cached AB must have zero native transport");
  return 0;
}
}
int main(int argc, char** argv) {
  if (argc == 2 && std::string_view(argv[1]) == "--released") return run(false);
  if (argc != 1) return dx12_fail(99, "expected no argument or --released");
  if (const auto held = run(true)) return held;
  return run(false);
}
