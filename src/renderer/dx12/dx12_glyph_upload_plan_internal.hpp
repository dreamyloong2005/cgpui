#pragma once
#include "dx12_glyph_suffix_internal.hpp"

namespace cgpui {
struct Dx12GlyphUploadChunk {
  Dx12GlyphRecordIndex source;
  GlyphAtlasUploadBatch batch;
  const std::uint8_t* source_alpha_data = nullptr; // Borrowed observation, never dereferenced after planning.
  const std::uint8_t* planner_alpha_data = nullptr; // Captures actual owned buffer before move handoffs.
};
struct Dx12GlyphUploadPlan {
  Dx12GlyphSnapshot target;
  std::size_t base_prefix = 0, full_bytes = 0, dirty_bytes = 0, planner_bytes = 0;
  std::vector<Dx12GlyphUploadChunk> chunks;
};
// Region adaptation only. Native owner must separately select completed-base/COW/full/hit.
Result<Dx12GlyphUploadPlan> dx12_plan_glyph_regions(
    const Dx12GlyphLineage&, const Dx12GlyphSnapshot&, std::size_t base_prefix,
    std::span<const GlyphUploadRecord>, std::span<const GlyphAtlasPage>);
} // namespace cgpui
