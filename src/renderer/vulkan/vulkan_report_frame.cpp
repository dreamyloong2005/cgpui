#include "vulkan_report_internal.hpp"

namespace cgpui {

RendererFrameReport vulkan_build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache) {
  return renderer_frame_report_from_command_report(
      vulkan_build_renderer_command_report(
          rects,
          rounded_rects,
          text_draws,
          selections,
          carets,
          glyph_cache));
}

RendererFrameReport vulkan_build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  return vulkan_build_renderer_frame_report(
      rects,
      std::span<const RoundedRectDraw>{},
      text_draws,
      std::span<const TextSelectionDraw>{},
      std::span<const TextCaretDraw>{},
      glyph_cache);
}

} // namespace cgpui
