#include "dx12_glyph_defensive_history_test_support.hpp"
#include <array>
#include <iostream>

using namespace cgpui;
using namespace cgpui_test;
namespace {
int run(bool overlapping) {
  std::array<Dx12TextureCopyEvent, 64> events{};
  Dx12TextureCopyObserver observer(events);
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "defensive dense fixture");
  state->texture_renderer->glyph_native->observer = &observer;
  GlyphDefensiveHistory history;
  auto a = history.append(1, 3, 2);
  if (!glyph_defensive_submit(*state, a, history)) return dx12_fail(2, "defensive completed base");
  auto* base = state->texture_renderer->native_textures.at(a.get()).resource.Get();
  auto old = glyph_readback(*state, *base);
  if (!old || *old != a->pixels) return dx12_fail(3, "defensive base readback");
  (void)history.append(2, 16, overlapping ? 16 : 8);
  auto target = history.append(3, 16, overlapping ? 16 : 8, overlapping ? 0 : 8);
  const auto sealed = history.lineage.find_snapshot(target);
  const auto plan = dx12_plan_glyph_regions(history.lineage, *sealed, 1, history.records, history.pages);
  const auto expected_d = overlapping ? 512U : 256U;
  if (!plan || plan->dirty_bytes != expected_d || plan->full_bytes != 256 || plan->chunks.size() != 2)
    return dx12_fail(4, "synthetic valid dense history did not reach D>=P");
  observer.clear();
  if (!glyph_defensive_submit(*state, target, history)) return dx12_fail(5, "native dense fallback");
  auto* native = state->texture_renderer->native_textures.at(target.get()).resource.Get();
  auto whole = glyph_readback(*state, *native);
  auto unchanged = glyph_readback(*state, *base);
  if (native == base || !whole || *whole != target->pixels || !unchanged || *unchanged != a->pixels ||
      glyph_copy_count(observer, Dx12TextureCopyKind::full_upload) != 1 ||
      glyph_copy_bytes(observer, Dx12TextureCopyKind::full_upload) != 256 ||
      glyph_copy_count(observer, Dx12TextureCopyKind::clone) ||
      glyph_copy_count(observer, Dx12TextureCopyKind::region_upload) || observer.overflowed())
    return dx12_fail(6, "valid dense input must actually full-initialize P without cloning");
  // Invalid history must not be hidden by the valid-D>=P full fallback.
  auto successor = history.append(4, 16, 16);
  history.records.back().alpha.pop_back();
  observer.clear();
  const auto denied = glyph_defensive_submit(*state, successor, history);
  if (denied || denied.error().code != ErrorCode::invalid_argument || !observer.events().empty() ||
      state->texture_renderer->native_textures.at(successor.get()).resource)
    return dx12_fail(7, "invalid dense source was disguised as full fallback");
  if (!dx12_check_debug_errors(*state)) return dx12_fail(8, "dense invalid-control Reset/Close debug gate");
  std::cout << "synthetic sealed-history native defensive fallback D=" << expected_d
            << " P=" << target->pixels.size() << " full-copy=observed; not normal GlyphCache packing\n";
  return 0;
}
}
int main() {
  if (const auto equal = run(false)) return equal;
  return run(true);
}
