#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::prepare_glyph_atlas_frame(
    std::span<const TextDraw> text_draws) {
  for (const TextDraw& text_draw : text_draws) {
    vulkan_consume_text_draw(text_draw, glyph_cache_);
  }
  const std::vector<GlyphAtlasUploadBatch> batches =
      vulkan_plan_glyph_atlas_uploads(
          glyph_cache_.upload_records(),
          glyph_cache_.atlas_pages());
  pending_glyph_atlas_plan_state_ = glyph_atlas_plan_state_;
  const GlyphAtlasProductionPlan plan =
      vulkan_plan_glyph_atlas_production_resources(
          pending_glyph_atlas_plan_state_,
          batches);

  vulkan_destroy_glyph_atlas_upload_resources(device_, glyph_atlas_uploads_);
  if (auto result = vulkan_update_glyph_atlas_resources(
          physical_device_,
          device_,
          plan,
          glyph_atlas_resources_);
      !result) {
    return result;
  }
  return vulkan_stage_glyph_atlas_uploads(
      physical_device_,
      device_,
      plan,
      batches,
      glyph_atlas_uploads_);
}

void VulkanRendererState::commit_glyph_atlas_frame() {
  vulkan_commit_glyph_atlas_uploads(
      glyph_atlas_resources_,
      glyph_atlas_uploads_);
  glyph_atlas_plan_state_ = std::move(pending_glyph_atlas_plan_state_);
}

} // namespace cgpui
