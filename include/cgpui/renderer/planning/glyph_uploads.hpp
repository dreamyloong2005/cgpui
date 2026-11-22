#pragma once

#include "cgpui/renderer/glyph_uploads.hpp"

namespace cgpui {

std::vector<GlyphAtlasUploadBatch> plan_glyph_atlas_uploads(
    std::span<const GlyphUploadRecord> upload_records,
    std::span<const GlyphAtlasPage> atlas_pages);

} // namespace cgpui
