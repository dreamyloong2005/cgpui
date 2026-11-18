#include "text_composition_paint.hpp"

#include "text_selection_paint_geometry.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {
namespace {

bool selected_clause(ImeClauseStyle style) {
  return style == ImeClauseStyle::selected_converted ||
         style == ImeClauseStyle::selected_unconverted;
}

// Underline of one clause on one line, snapped to device pixels. The font's
// underline position and thickness are used when the face reports them.
Rect clause_underline(const TextRangeLineRect& line, bool thick,
                      float font_size, float scale) {
  const TextFontMetrics& metrics = line.font_metrics;
  const float thin_device = std::max(
      1.0F, std::round(metrics.underline_thickness * scale));
  const float height = (thick ? 2.0F : 1.0F) * thin_device / scale;
  const float position = metrics.underline_position > 0.0F
      ? metrics.underline_position
      : font_size * 0.1F;
  const float bottom = line.rect.origin.y + line.rect.size.height;
  const float top = std::min(
      std::round((line.baseline + position) * scale) / scale,
      bottom - height);
  // One device pixel gap at the clause end keeps adjacent clauses apart.
  const float gap = line.rect.size.width > 2.0F / scale ? 1.0F / scale : 0.0F;
  return Rect{
      .origin = {.x = line.rect.origin.x, .y = top},
      .size = {.width = line.rect.size.width - gap, .height = height},
  };
}

} // namespace

TextCompositionDisplay text_composition_display(const TextModel& model) {
  const TextSelectionRange selection = model.selection();
  const std::string_view text = model.text();
  const TextComposition& composition = model.composition();
  TextCompositionDisplay display;
  display.text.reserve(text.size() + composition.text.size());
  display.text.append(text.substr(0, selection.start));
  display.text.append(composition.text);
  display.text.append(text.substr(selection.end));
  display.composition_start = selection.start;
  display.caret = selection.start + composition.caret_end;
  return display;
}

void paint_text_composition(
    PaintList& paint_list,
    Rect bounds,
    Color color,
    const TextModel& model,
    const FontDescriptor& font,
    float font_size) {
  const DpiScale scale = paint_list.scale();
  const float device_scale = scale.value > 0.0F ? scale.value : 1.0F;
  const TextComposition& composition = model.composition();
  const TextCompositionDisplay display = text_composition_display(model);
  const std::size_t start = display.composition_start;
  if (composition.caret_visible &&
      composition.caret_begin < composition.caret_end) {
    paint_text_selection_ranges(
        paint_list, bounds,
        Color{.r = 0.22F, .g = 0.42F, .b = 0.80F, .a = 0.38F},
        display.text, font, font_size, scale,
        TextSelectionRange{.start = start + composition.caret_begin,
                           .end = start + composition.caret_end,
                           .collapsed = false});
  }
  if (!display.text.empty()) {
    paint_list.fill_text(bounds, color, display.text, font, font_size);
  }
  for (const TextCompositionClause& clause : composition.clauses) {
    const std::size_t clause_start = start + clause.byte_offset;
    for (const TextRangeLineRect& line : text_range_line_rects(
             bounds, display.text, font, font_size, scale, clause_start,
             clause_start + clause.byte_length)) {
      paint_list.fill_rect(
          clause_underline(line, selected_clause(clause.style), font_size,
                           device_scale),
          color);
    }
  }
  if (composition.caret_visible) {
    paint_text_caret_geometry(paint_list, bounds, color, display.text, font,
                              font_size, scale, display.caret);
  }
}

} // namespace cgpui
