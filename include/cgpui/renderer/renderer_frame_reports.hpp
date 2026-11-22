#pragma once

#include "cgpui/renderer/glyph_cache.hpp"
#include "cgpui/renderer/renderer_geometry_reports.hpp"
#include "cgpui/renderer/renderer_image_reports.hpp"
#include "cgpui/renderer/renderer_submission_reports.hpp"

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace cgpui {

struct RendererCommandReport {
  std::vector<RendererCommandBatch> batches;
  std::vector<RendererSubmissionPlanRecord> submission_plan_records;
  std::vector<RendererUnsupportedCommandDiagnostic> unsupported_commands;
  std::vector<RoundedRectTessellationRecord> rounded_rect_tessellations;
  std::vector<TextSelectionGeometryRecord> text_selection_geometries;
  std::vector<TextCaretGeometryRecord> text_caret_geometries;
  std::size_t supported_command_count = 0;
  std::size_t unsupported_command_count = 0;
  std::size_t submission_plan_record_count = 0;
  std::size_t submission_plan_command_count = 0;
  std::size_t submission_plan_glyph_quad_count = 0;
  std::size_t rounded_rect_tessellation_count = 0;
  std::size_t text_selection_geometry_count = 0;
  std::size_t text_caret_geometry_count = 0;
  std::size_t clip_stack_record_count = 0;
  std::size_t max_clip_stack_depth = 0;
  std::size_t composition_stack_record_count = 0;
  std::size_t max_composition_stack_depth = 0;
  RendererTextRenderReport text_render;
  RendererImageRenderReport image_render;

  [[nodiscard]] std::size_t command_count() const {
    return supported_command_count + unsupported_command_count;
  }
};

enum class RendererFrameGapKind {
  unsupported_command,
  pending_text_sampler_pipeline,
  metadata_only_text_draw,
  missing_submission_plan,
};

struct RendererFrameGap {
  RendererFrameGapKind kind = RendererFrameGapKind::unsupported_command;
  RendererPrimitiveKind primitive_kind = RendererPrimitiveKind::solid_rect;
  std::size_t count = 0;
  std::string message;
};

struct RendererFrameReport {
  RendererCommandReport command_report;
  std::size_t supported_primitive_count = 0;
  std::size_t unsupported_primitive_count = 0;
  std::size_t renderer_batch_count = 0;
  std::size_t submission_plan_record_count = 0;
  std::size_t glyph_upload_record_count = 0;
  std::size_t textured_glyph_quad_count = 0;
  std::size_t image_upload_plan_count = 0;
  std::size_t image_upload_byte_count = 0;
  std::size_t gap_count = 0;
  std::vector<RendererFrameGap> gaps;

  [[nodiscard]] bool has_gaps() const { return gap_count > 0; }
};

[[nodiscard]] RendererFrameReport
renderer_frame_report_from_command_report(
    const RendererCommandReport& command_report);

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const RendererCommandStreamItem> commands);
RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache);
RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache);
RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache);
RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    std::span<const ImageDraw> image_draws,
    GlyphCache& glyph_cache);
RendererFrameReport vulkan_build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache);
RendererFrameReport vulkan_build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache);
std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws);
std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws);
std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets);

} // namespace cgpui
