#pragma once
#include "dx12_glyph_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_native_recording_internal.hpp"

namespace cgpui_test {
// Deliberately synthetic coherent sealed history, not normal nonoverlapping GlyphCache packing.
struct GlyphDefensiveHistory {
  cgpui::Dx12GlyphLineage lineage;
  std::vector<cgpui::GlyphUploadRecord> records;
  std::vector<cgpui::GlyphAtlasPage> pages{{.page_index = 0, .size = {16, 16}}};
  std::vector<std::uint8_t> alpha = std::vector<std::uint8_t>(256);
  GlyphDefensiveHistory();
  cgpui::Dx12TextureHandle append(unsigned id, unsigned width, unsigned height, unsigned y = 0);
  cgpui::Dx12GlyphTargetInputs inputs() const;
};
// Uses the same private preparation/dispatch + stage/record/commit body as production.
// Resource construction is fixture-owned; no public glyph_add reachability or frame capture claim.
cgpui::Result<void> glyph_defensive_submit(cgpui::Dx12State&,
    const cgpui::Dx12TextureHandle&, const GlyphDefensiveHistory&);
} // namespace cgpui_test
