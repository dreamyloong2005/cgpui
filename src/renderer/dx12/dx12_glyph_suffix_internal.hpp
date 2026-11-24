#pragma once
#include "dx12_glyph_lineage_internal.hpp"
#include "cgpui/renderer/glyph_uploads.hpp"
#include <vector>

namespace cgpui {
struct Dx12GlyphRecordIndex {
  std::size_t ordinal = 0, global_index = 0;
};
struct Dx12GlyphSuffix {
  Dx12GlyphSnapshot target;
  std::size_t base_prefix = 0, full_bytes = 0, dirty_bytes = 0, planner_bytes = 0;
  std::vector<Dx12GlyphRecordIndex> records;
};
Result<Dx12GlyphSuffix> dx12_glyph_suffix(
    const Dx12GlyphLineage&, const Dx12GlyphSnapshot&, std::size_t base_prefix,
    std::span<const GlyphUploadRecord>, std::span<const GlyphAtlasPage>);
} // namespace cgpui
