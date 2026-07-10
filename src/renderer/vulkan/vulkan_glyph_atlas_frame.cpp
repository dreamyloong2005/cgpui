#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::prepare_glyph_atlas_frame(
    std::span<const TextDraw> text_draws) {
  VulkanGlyphAtlasDrawData draw_data =
      vulkan_plan_glyph_atlas_draw_data(text_draws, glyph_cache_);
  const std::vector<GlyphAtlasUploadBatch> batches =
      vulkan_plan_glyph_atlas_uploads(
          glyph_cache_.upload_records(),
          glyph_cache_.atlas_pages());
  pending_frame_upload_bytes_ = vulkan_merge_upload_byte_counts(
      renderer_upload_byte_counts(batches, {}),
      pending_frame_upload_bytes_);
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
  if (auto result = vulkan_stage_glyph_atlas_uploads(
          physical_device_,
          device_,
          plan,
          batches,
          glyph_atlas_uploads_);
      !result) {
    return result;
  }
  auto bindings = vulkan_resolve_glyph_atlas_draw_bindings(
      draw_data.page_usages,
      glyph_atlas_resources_);
  if (!bindings) {
    vulkan_destroy_glyph_atlas_upload_resources(device_, glyph_atlas_uploads_);
    return std::unexpected(bindings.error());
  }
  glyph_atlas_draw_quads_ = std::move(draw_data.quads);
  glyph_atlas_draw_bindings_ = std::move(*bindings);
  if (auto result = vulkan_upload_text_vertex_buffer(
          physical_device_,
          device_,
          glyph_atlas_draw_quads_,
          vulkan_default_text_positioning_policy(),
          text_vertex_buffer_);
      !result) {
    vulkan_destroy_glyph_atlas_upload_resources(device_, glyph_atlas_uploads_);
    return result;
  }
  return {};
}

void VulkanRendererState::commit_glyph_atlas_frame() {
  vulkan_commit_glyph_atlas_uploads(
      glyph_atlas_resources_,
      glyph_atlas_uploads_);
  glyph_atlas_plan_state_ = std::move(pending_glyph_atlas_plan_state_);
}

} // namespace cgpui
