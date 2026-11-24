#include "dx12_glyph_cpu_test_support.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;
int main() {
  try {
    GlyphCpuHistory fixture;
    auto plan = dx12_plan_glyph_regions(fixture.lineage, fixture.sealed, 2, fixture.records, fixture.pages);
    glyph_require(bool(plan) && plan->chunks.size() == 2, "fresh-per-record state lost second region");
    glyph_require(plan->full_bytes == 256 && plan->dirty_bytes == 21 && plan->planner_bytes == 29,
                  "independent P/D/Q ledger mismatch");
    for (std::size_t i = 0; i < 2; ++i) {
      const auto& chunk = plan->chunks[i];
      glyph_require(chunk.source.ordinal == i + 3 && chunk.source.global_index == i + 7,
                    "global indices mistaken for page-local ordinals");
      glyph_require(chunk.batch.alpha == fixture.records[i + 7].alpha &&
          chunk.batch.uploads[0].byte_offset == 0 && chunk.batch.uploads[0].atlas_bounds.origin.x == (i ? 7 : 1),
          "odd-width padded region coverage/origin changed");
    }
    glyph_require(!dx12_plan_glyph_regions(fixture.lineage, fixture.sealed, 4, fixture.records, fixture.pages),
                  "empty/backward suffix accepted");
    glyph_require(!dx12_plan_glyph_regions(fixture.lineage, fixture.sealed, 2, fixture.records, {}),
                  "missing page silently used shared planner fallback");
    auto pages = fixture.pages; pages.push_back(pages[0]);
    glyph_require(!dx12_plan_glyph_regions(fixture.lineage, fixture.sealed, 2, fixture.records, pages),
                  "duplicate page descriptor");
    for (unsigned mutation = 0; mutation < 6; ++mutation) {
      auto records = fixture.records;
      auto& record = records[8];
      if (mutation == 0) record.stride = 4;
      if (mutation == 1) record.alpha.resize(20); // required last row end is21.
      if (mutation == 2) record.atlas_bounds.origin.x = std::numeric_limits<float>::quiet_NaN();
      if (mutation == 3) record.key.glyph_id = 999;
      if (mutation == 4) record.page_index = 1;
      if (mutation == 5) record.width = std::numeric_limits<unsigned>::max();
      glyph_require(!dx12_plan_glyph_regions(fixture.lineage, fixture.sealed, 2, records, fixture.pages),
                    "invalid source record accepted");
    }
    Dx12GlyphLineage gapped;
    glyph_require(bool(gapped.ensure_page(0, 16, 16)), "gap family");
    for (auto global : {0U, 7U, 8U}) { // Deliberately omit page0's global3.
      glyph_require(bool(gapped.prepare_record(0, global)), "gap prepare");
      gapped.commit_record(0, global);
    }
    glyph_require(bool(gapped.seal_snapshot(0, fixture.snapshot)), "gap seal");
    glyph_require(!dx12_glyph_suffix(gapped, *gapped.find_snapshot(fixture.snapshot), 1,
                                    fixture.records, fixture.pages), "gapped page-local lineage accepted");
    auto newer = fixture.sealed; ++newer.prefix;
    glyph_require(!dx12_glyph_suffix(fixture.lineage, newer, 2, fixture.records, fixture.pages),
                  "unsealed future record accepted");
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
