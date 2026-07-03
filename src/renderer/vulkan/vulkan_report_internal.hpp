#pragma once

#include "cgpui/renderer/renderer.hpp"

#include <optional>
#include <span>
#include <vector>

namespace cgpui {

struct VulkanTextDrawAtlasPageUsage {
  std::size_t page_index = 0;
  std::size_t glyph_quad_count = 0;
};

[[nodiscard]] std::vector<RendererCommandStreamItem>
vulkan_build_renderer_command_stream(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    std::span<const ImageDraw> image_draws);

void vulkan_record_renderer_text_statistics(
    RendererCommandReport& report,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache,
    std::span<std::vector<VulkanTextDrawAtlasPageUsage>>
        text_draw_atlas_pages);

void vulkan_record_renderer_image_statistics(
    RendererCommandReport& report,
    std::span<const ImageDraw> image_draws);

void vulkan_append_text_draw_atlas_page_usage(
    std::vector<VulkanTextDrawAtlasPageUsage>& usages,
    std::size_t page_index);

void vulkan_append_submission_plan_record(
    std::vector<RendererSubmissionPlanRecord>& records,
    RendererSubmissionPlanKey key,
    TextSamplerPipelineDescriptor pipeline,
    std::size_t batch_index,
    const RendererCommandBatch& batch,
    std::size_t glyph_quad_count);

[[nodiscard]] bool vulkan_append_text_submission_plan_records(
    std::vector<RendererSubmissionPlanRecord>& records,
    std::size_t batch_index,
    const RendererCommandBatch& batch,
    std::span<const std::vector<VulkanTextDrawAtlasPageUsage>>
        text_draw_atlas_pages,
    TextSamplerPipelineDescriptor pipeline);

[[nodiscard]] std::vector<RendererSubmissionPlanRecord>
vulkan_build_renderer_submission_plan(
    std::span<const RendererCommandBatch> batches,
    std::span<const std::vector<VulkanTextDrawAtlasPageUsage>>
        text_draw_atlas_pages,
    const TextSamplerPipelineDescriptor& text_sampler_pipeline);

void vulkan_record_submission_plan_statistics(RendererCommandReport& report);

[[nodiscard]] bool vulkan_same_rect(Rect lhs, Rect rhs);

[[nodiscard]] bool vulkan_same_clip_rect(
    const std::optional<Rect>& lhs,
    const std::optional<Rect>& rhs);

[[nodiscard]] bool vulkan_same_clip_stack(
    const RendererClipStackRecord& lhs,
    const RendererClipStackRecord& rhs);

[[nodiscard]] bool vulkan_same_command_batch_key(
    const RendererCommandBatchKey& lhs,
    const RendererCommandBatchKey& rhs);

[[nodiscard]] bool vulkan_same_submission_plan_key(
    const RendererSubmissionPlanKey& lhs,
    const RendererSubmissionPlanKey& rhs);

[[nodiscard]] bool vulkan_is_supported_renderer_primitive(
    RendererPrimitiveKind primitive_kind);

[[nodiscard]] RendererUnsupportedCommandDiagnostic
vulkan_make_unsupported_renderer_command_diagnostic(
    const RendererCommandStreamItem& command);

} // namespace cgpui
