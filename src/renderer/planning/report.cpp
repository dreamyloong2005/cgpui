#include "report_internal.hpp"

namespace cgpui {

RendererCommandReport build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache) {
  return build_renderer_command_report(
      rects,
      rounded_rects,
      text_draws,
      selections,
      carets,
      std::span<const ImageDraw>{},
      glyph_cache);
}

RendererCommandReport build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    std::span<const ImageDraw> image_draws,
    GlyphCache& glyph_cache) {
  const std::vector<RendererCommandStreamItem> commands =
      renderer_plan_build_renderer_command_stream(
          rects,
          rounded_rects,
          text_draws,
          selections,
          carets,
          image_draws);
  RendererCommandReport report = build_renderer_command_report(commands);
  report.rounded_rect_tessellations =
      tessellate_rounded_rects(rounded_rects);
  report.rounded_rect_tessellation_count =
      report.rounded_rect_tessellations.size();
  report.text_selection_geometries =
      build_text_selection_geometry(selections);
  report.text_selection_geometry_count =
      report.text_selection_geometries.size();
  report.text_caret_geometries = build_text_caret_geometry(carets);
  report.text_caret_geometry_count = report.text_caret_geometries.size();

  std::vector<std::vector<RendererTextDrawAtlasPageUsage>> text_draw_atlas_pages(
      text_draws.size());
  renderer_plan_record_renderer_text_statistics(
      report,
      text_draws,
      glyph_cache,
      text_draw_atlas_pages);
  renderer_plan_record_renderer_image_statistics(report, image_draws);

  report.submission_plan_records = renderer_plan_build_renderer_submission_plan(
      report.batches,
      text_draw_atlas_pages,
      report.text_render.text_sampler_pipeline);
  renderer_plan_record_submission_plan_statistics(report);
  return report;
}

RendererCommandReport build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  return build_renderer_command_report(
      rects,
      rounded_rects,
      text_draws,
      std::span<const TextSelectionDraw>{},
      std::span<const TextCaretDraw>{},
      glyph_cache);
}

RendererCommandReport build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  return build_renderer_command_report(
      rects,
      std::span<const RoundedRectDraw>{},
      text_draws,
      std::span<const TextSelectionDraw>{},
      std::span<const TextCaretDraw>{},
      glyph_cache);
}

} // namespace cgpui
