#include "cgpui/renderer/planning/command_reports.hpp"

namespace cgpui {

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const RendererCommandStreamItem> commands) {
  return build_renderer_command_report(commands);
}

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  return build_renderer_command_report(rects, text_draws, glyph_cache);
}

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  return build_renderer_command_report(rects, rounded_rects, text_draws, glyph_cache);
}

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache) {
  return build_renderer_command_report(rects, rounded_rects, text_draws, selections, carets, glyph_cache);
}

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    std::span<const ImageDraw> image_draws,
    GlyphCache& glyph_cache) {
  return build_renderer_command_report(rects, rounded_rects, text_draws, selections, carets, image_draws, glyph_cache);
}

RendererFrameReport vulkan_build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  return build_renderer_frame_report(rects, text_draws, glyph_cache);
}

RendererFrameReport vulkan_build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache) {
  return build_renderer_frame_report(rects, rounded_rects, text_draws, selections, carets, glyph_cache);
}

std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws) {
  return build_renderer_command_batches(rects, text_draws);
}

std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws) {
  return build_renderer_command_batches(rects, rounded_rects, text_draws);
}

std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets) {
  return build_renderer_command_batches(rects, rounded_rects, text_draws, selections, carets);
}

} // namespace cgpui
