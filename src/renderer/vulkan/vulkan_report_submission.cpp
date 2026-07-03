#include "vulkan_report_internal.hpp"

#include <cstddef>
#include <utility>
namespace cgpui {
namespace {

[[nodiscard]] TextSamplerPipelineDescriptor submission_pipeline_for(
    RendererPrimitiveKind primitive_kind,
    const TextSamplerPipelineDescriptor& text_sampler_pipeline) {
  if (primitive_kind == RendererPrimitiveKind::text) {
    return text_sampler_pipeline;
  }
  return TextSamplerPipelineDescriptor{
      .primitive_kind = primitive_kind,
      .uses_alpha_sampling = false,
      .uses_text_color = true,
  };
}

} // namespace

void vulkan_append_submission_plan_record(
    std::vector<RendererSubmissionPlanRecord>& records, RendererSubmissionPlanKey key,
    TextSamplerPipelineDescriptor pipeline, std::size_t batch_index,
    const RendererCommandBatch& batch, std::size_t glyph_quad_count) {
  if (records.empty() ||
      !vulkan_same_submission_plan_key(records.back().key, key) ||
      records.back().pipeline != pipeline) {
    records.push_back(RendererSubmissionPlanRecord{
        .key = std::move(key),
        .pipeline = pipeline,
    });
  }

  RendererSubmissionPlanRecord& record = records.back();
  record.batch_count += 1;
  record.command_count += batch.command_count;
  record.glyph_quad_count += glyph_quad_count;
  record.batch_indices.push_back(batch_index);
  record.command_indices.insert(
      record.command_indices.end(),
      batch.command_indices.begin(),
      batch.command_indices.end());
}

std::vector<RendererSubmissionPlanRecord>
vulkan_build_renderer_submission_plan(
    std::span<const RendererCommandBatch> batches,
    std::span<const std::vector<VulkanTextDrawAtlasPageUsage>>
        text_draw_atlas_pages,
    const TextSamplerPipelineDescriptor& text_sampler_pipeline) {
  std::vector<RendererSubmissionPlanRecord> records;
  records.reserve(batches.size());

  for (std::size_t batch_index = 0; batch_index < batches.size();
       ++batch_index) {
    const RendererCommandBatch& batch = batches[batch_index];
    const TextSamplerPipelineDescriptor pipeline = submission_pipeline_for(
        batch.key.primitive_kind,
        text_sampler_pipeline);
    if (batch.key.primitive_kind != RendererPrimitiveKind::text) {
      vulkan_append_submission_plan_record(
          records,
          RendererSubmissionPlanKey{
              .primitive_kind = batch.key.primitive_kind,
              .clip_rect = batch.key.clip_rect,
              .clip_stack = batch.key.clip_stack,
          },
          pipeline,
          batch_index,
          batch,
          0);
      continue;
    }

    if (!vulkan_append_text_submission_plan_records(
            records,
            batch_index,
            batch,
            text_draw_atlas_pages,
            pipeline)) {
      vulkan_append_submission_plan_record(
          records,
          RendererSubmissionPlanKey{
              .primitive_kind = batch.key.primitive_kind,
              .clip_rect = batch.key.clip_rect,
              .clip_stack = batch.key.clip_stack,
          },
          pipeline,
          batch_index,
          batch,
          0);
    }
  }

  return records;
}

} // namespace cgpui
