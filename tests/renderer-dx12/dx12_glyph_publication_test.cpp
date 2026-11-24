#include "dx12_glyph_cpu_test_support.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include "../../src/renderer/dx12/dx12_text_cache_internal.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;
namespace {
RasterizedGlyph glyph(unsigned id) {
  const auto record = glyph_cpu_record(id, 0, 0, 0);
  return {.key = record.key, .bitmap = {.width = record.width, .height = record.height,
                                      .stride = record.stride, .alpha = record.alpha}};
}
}
int main() {
  try {
    Dx12State state; // CPU construction only: no create_device, HWND, command list or submission.
    Dx12TextCache cache;
    (void)cache.glyphs.allocate(glyph(1));
    glyph_require(bool(dx12_update_glyph_pages(state, cache)), "first CPU publication");
    auto a = cache.pages[0].texture;
    const auto a_bytes = a->pixels;
    std::weak_ptr<const Dx12Texture> weak_a = a;
    const auto a_seal = *cache.lineage.find_snapshot(a);
    (void)cache.glyphs.allocate(glyph(2));
    (void)cache.glyphs.allocate(glyph(3));
    glyph_require(bool(dx12_update_glyph_pages(state, cache)), "CPU successor publication");
    const auto ab = *cache.lineage.find_snapshot(cache.pages[0].texture);
    glyph_require(a_seal.prefix == 1 && ab.prefix == 3 && a_seal.family == ab.family &&
        a->pixels == a_bytes && a != cache.pages[0].texture && !cache.pages[0].dirty,
        "publication mutated old snapshot or sealed wrong prefix");
    auto suffix = dx12_plan_glyph_regions(cache.lineage, ab, 1, cache.glyphs.upload_records(),
                                          cache.glyphs.atlas_pages());
    glyph_require(suffix && suffix->chunks.size() == 2 && suffix->dirty_bytes == 12,
                  "production CPU publication did not expose exact two-record suffix");
    a.reset();
    glyph_require(weak_a.expired(), "CPU lineage/plan pinned predecessor");
    for (unsigned id = 4; id < 40; ++id) {
      (void)cache.glyphs.allocate(glyph(id));
      glyph_require(bool(dx12_update_glyph_pages(state, cache)), "abandoned CPU update");
      glyph_require(cache.lineage.stats().weak_snapshots == 1 &&
          cache.lineage.stats().record_indices == cache.processed_uploads,
          "no-Present publication accumulated expired versions or duplicate history");
    }
    Dx12TextCache rejected;
    auto invalid = glyph(99);
    invalid.bitmap.alpha.resize(1);
    (void)rejected.glyphs.allocate(invalid);
    auto result = dx12_update_glyph_pages(state, rejected);
    glyph_require(!result && result.error().code == ErrorCode::invalid_argument &&
        rejected.processed_uploads == 0 && rejected.lineage.stats().record_indices == 0 &&
        !rejected.pages[0].texture && rejected.pages[0].dirty,
        "invalid upload advanced progress or published an unsealed snapshot");
    glyph_require(!state.device && !state.commands, "CPU test unexpectedly created native device/list");
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
