#pragma once

#include "cgpui/renderer/glyph_texture_resources.hpp"
#include "cgpui/renderer/glyph_atlas_production.hpp"

namespace cgpui {

GlyphAtlasTextureResourcePlan update_glyph_atlas_texture_resources(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches);
std::vector<GlyphAtlasDirtyUploadRange> plan_glyph_atlas_dirty_uploads(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches);
GlyphAtlasProductionPlan plan_glyph_atlas_production_resources(
    GlyphAtlasProductionResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches);

} // namespace cgpui
