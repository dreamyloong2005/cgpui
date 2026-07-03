#include "vulkan_report_internal.hpp"

namespace cgpui {

void vulkan_append_text_draw_atlas_page_usage(
    std::vector<VulkanTextDrawAtlasPageUsage>& usages,
    std::size_t page_index) {
  for (VulkanTextDrawAtlasPageUsage& usage : usages) {
    if (usage.page_index == page_index) {
      usage.glyph_quad_count += 1;
      return;
    }
  }
  usages.push_back(VulkanTextDrawAtlasPageUsage{
      .page_index = page_index,
      .glyph_quad_count = 1,
  });
}

bool vulkan_append_text_submission_plan_records(
    std::vector<RendererSubmissionPlanRecord>& records,
    std::size_t batch_index,
    const RendererCommandBatch& batch,
    std::span<const std::vector<VulkanTextDrawAtlasPageUsage>>
        text_draw_atlas_pages,
    TextSamplerPipelineDescriptor pipeline) {
  bool appended_text_page = false;
  for (const std::size_t command_index : batch.command_indices) {
    if (command_index >= text_draw_atlas_pages.size()) {
      continue;
    }
    for (const VulkanTextDrawAtlasPageUsage& usage :
         text_draw_atlas_pages[command_index]) {
      vulkan_append_submission_plan_record(
          records,
          RendererSubmissionPlanKey{
              .primitive_kind = batch.key.primitive_kind,
              .clip_rect = batch.key.clip_rect,
              .clip_stack = batch.key.clip_stack,
              .atlas_page_index = usage.page_index,
          },
          pipeline,
          batch_index,
          RendererCommandBatch{
              .key = batch.key,
              .command_count = 1,
              .command_indices = {command_index},
          },
          usage.glyph_quad_count);
      appended_text_page = true;
    }
  }
  return appended_text_page;
}

} // namespace cgpui
