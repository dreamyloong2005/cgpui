#include "cgpui/renderer/planning/glyph_uploads.hpp"

namespace cgpui {

std::vector<GlyphAtlasUploadBatch> vulkan_plan_glyph_atlas_uploads(
    std::span<const GlyphUploadRecord> upload_records,
    std::span<const GlyphAtlasPage> atlas_pages) {
  return plan_glyph_atlas_uploads(upload_records, atlas_pages);
}

} // namespace cgpui
