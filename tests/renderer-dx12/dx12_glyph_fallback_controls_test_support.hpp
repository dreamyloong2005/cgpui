#pragma once
#include "dx12_glyph_native_test_support.hpp"
#include "dx12_glyph_cpu_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_native_recording_internal.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace cgpui_test {
bool glyph_fallback_enable_debug(); // Process-local, before its first device; preserves GPU-validation/adapter selection.
// Fixture-owned valid records and immutable snapshots; not GlyphCache shelf packing.
struct GlyphFallbackHistory {
  cgpui::Dx12GlyphLineage lineage;
  std::vector<cgpui::GlyphUploadRecord> records;
  std::vector<cgpui::GlyphAtlasPage> pages{{.page_index = 0, .size = {16, 16}},
                                        {.page_index = 1, .size = {16, 16}}};
  std::array<std::vector<std::uint8_t>, 2> alpha;
  GlyphFallbackHistory();
  cgpui::Dx12TextureHandle append(unsigned page, unsigned id, unsigned x, unsigned y);
  cgpui::Dx12GlyphTargetInputs inputs() const;
};
// Fresh-target-only private input-boundary control. Calls unchanged production
// prepare/stage/record; native target creation belongs to this fixture, not an oracle event.
cgpui::Result<void> glyph_fallback_submit(cgpui::Dx12State&, const cgpui::Dx12TextureHandle&,
    const cgpui::Dx12GlyphLineage& recording_lineage, const cgpui::Dx12GlyphTargetInputs&);
bool glyph_fallback_full_copy(const cgpui::Dx12TextureCopyObserver&,
                             ID3D12Resource&, const cgpui::Dx12Texture&);
cgpui::Result<void> glyph_fallback_readback(cgpui::Dx12State&, ID3D12Resource&,
                                         const cgpui::Dx12Texture& expected);
} // namespace cgpui_test
