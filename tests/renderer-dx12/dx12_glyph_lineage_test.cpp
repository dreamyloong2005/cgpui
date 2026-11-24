#include "dx12_glyph_cpu_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  try {
    Dx12GlyphLineage lineage;
    glyph_require(bool(lineage.ensure_page(0, 16, 16)), "initial family");
    glyph_require(!lineage.ensure_page(0, 17, 16) && !lineage.ensure_page(2, 16, 16), "extent/index mismatch");
    for (auto global : {0U, 3U}) {
      glyph_require(bool(lineage.prepare_record(0, global)), "reserve record");
      lineage.commit_record(0, global);
    }
    auto wrong_extent = glyph_cpu_snapshot(17, 16);
    glyph_require(!lineage.seal_snapshot(0, wrong_extent), "snapshot extent mismatch");
    auto a = glyph_cpu_snapshot();
    std::weak_ptr<const Dx12Texture> weak_a = a;
    glyph_require(bool(lineage.seal_snapshot(0, a)), "seal A");
    auto sealed_a = *lineage.find_snapshot(a);
    glyph_require(sealed_a.prefix == 2 && lineage.stats().record_indices == 2, "page-local prefix");
    glyph_require(!lineage.prepare_record(0, 3) && !lineage.prepare_record(0, 2), "duplicate/backward index");
    for (auto global : {7U, 8U}) {
      glyph_require(bool(lineage.prepare_record(0, global)), "reserve successor");
      lineage.commit_record(0, global);
    }
    glyph_require(!lineage.seal_snapshot(0, a), "cannot reseal immutable A as successor");
    glyph_require(lineage.find_snapshot(a)->prefix == 2, "rejected reseal changed A");
    auto ab = glyph_cpu_snapshot();
    glyph_require(bool(lineage.seal_snapshot(0, ab)), "seal AB");
    auto sealed_ab = *lineage.find_snapshot(ab);
    glyph_require(sealed_ab.prefix == 4 && sealed_ab.family == sealed_a.family, "family continuity");
    auto old_indices = lineage.record_indices(sealed_a);
    glyph_require(old_indices && old_indices->size() == 2 && (*old_indices)[1] == 3, "sealed old prefix grew");
    auto forged = sealed_ab; --forged.prefix;
    glyph_require(!lineage.record_indices(forged), "unpublished prefix accepted");
    a.reset();
    glyph_require(weak_a.expired(), "lineage took strong CPU ownership");
    lineage.prune_expired();
    glyph_require(lineage.stats().weak_snapshots == 1 && !lineage.record_indices(sealed_a), "expired sidecar leaked");
    const auto old_family = sealed_ab.family;
    lineage.clear();
    glyph_require(!lineage.current(sealed_ab) && lineage.stats().record_indices == 0, "clear did not retire family");
    glyph_require(bool(lineage.ensure_page(0, 16, 16)) && bool(lineage.seal_snapshot(0, ab)), "recycled page");
    glyph_require(lineage.find_snapshot(ab)->family != old_family, "page-index family ABA");
    ab.reset(); lineage.prune_expired();
    for (unsigned i = 0; i < 100; ++i) {
      {
        Dx12GlyphLineageSweep sweep(lineage);
        auto abandoned = glyph_cpu_snapshot();
        glyph_require(bool(lineage.seal_snapshot(0, abandoned)), "abandoned publication");
      }
      glyph_require(lineage.stats().weak_snapshots == 0, "no-Present sweep accumulated versions");
    }
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
