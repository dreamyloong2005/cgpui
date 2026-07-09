#include "ui_internal.hpp"

namespace cgpui {

void PaintList::fill_rich_text(
    Rect bounds,
    Color color,
    std::string_view text,
    std::span<const RichTextRun> rich_text_runs,
    std::span<const RichTextInlineImageRun> inline_images,
    FontDescriptor font,
    float font_size) {
  const TextMeasurement measurement =
      text_measurement_cache_ != nullptr
      ? text_measurement_cache_->measure(text, font, font_size, scale_)
            .measurement
      : measure_text(text, font, font_size, scale_);
  const TextShapeRun& shape_run = measurement.shape_run;
  const TextWrapLayout wrap_layout =
      wrap_text_measurement(measurement, bounds.size.width);
  Rect text_bounds = bounds;
  text_bounds.size.height =
      std::max(text_bounds.size.height, wrap_layout.logical_size.height);
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::text,
      .text =
          TextPaint{
              .bounds = text_bounds,
              .color = color,
              .font = shape_run.font,
              .content = std::string(text),
              .byte_length = text.size(),
              .font_size = font_size,
              .scale = scale_,
              .device_font_size = shape_run.device_font_size,
              .glyphs = text_glyph_paint_metadata(
                  shape_run,
                  wrap_layout.lines,
                  bounds.origin),
              .lines = wrap_layout.lines,
              .rich_text_runs = std::vector<RichTextRun>(
                  rich_text_runs.begin(),
                  rich_text_runs.end()),
              .rich_text_inline_images =
                  std::vector<RichTextInlineImageRun>(
                      inline_images.begin(),
                      inline_images.end()),
          },
      .clip_rect = current_clip_rect_for(clip_stack_),
      .clip_stack = clip_stack_record_for(clip_stack_),
      .composition_stack = composition_stack_record_for(metadata_stack_),
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

} // namespace cgpui
