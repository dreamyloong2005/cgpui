#include "cgpui/renderer/planning/glyph_resources.hpp"

namespace cgpui {

GlyphAtlasTextureResourcePlan vulkan_update_glyph_atlas_texture_resources(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches) {
  return update_glyph_atlas_texture_resources(state, upload_batches);
}

std::vector<GlyphAtlasDirtyUploadRange> vulkan_plan_glyph_atlas_dirty_uploads(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches) {
  return plan_glyph_atlas_dirty_uploads(state, upload_batches);
}

GlyphAtlasProductionPlan vulkan_plan_glyph_atlas_production_resources(
    GlyphAtlasProductionResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches) {
  return plan_glyph_atlas_production_resources(state, upload_batches);
}

} // namespace cgpui
