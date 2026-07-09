#include "ui_internal.hpp"

#include "text_style_inheritance.hpp"
#include "text_selection_paint_geometry.hpp"

namespace cgpui {

void LabelElement::inherit_text_style(const Style& style) {
  effective_style_ = merge_inherited_text_style(style, style_);
}

void LabelElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || text().empty()) {
    return;
  }
  const Style& text_style = effective_style();
  const Color text_color = text_style.foreground_color.value_or(
      Color{.r = 0.82F, .g = 0.86F, .b = 0.92F, .a = 1.0F});
  paint_list.push_metadata(paint_metadata_for_style(text_style));
  paint_list.fill_text(*bounds, text_color, text(), text_style.font, font_size());
  paint_list.pop_metadata();
}

void TextElement::inherit_text_style(const Style& style) {
  effective_style_ = merge_inherited_text_style(style, style_);
}

void TextElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || model_ == nullptr) {
    return;
  }
  const Style& text_style = effective_style();
  const Color text_color = text_style.foreground_color.value_or(
      Color{.r = 0.82F, .g = 0.86F, .b = 0.92F, .a = 1.0F});
  paint_list.push_metadata(paint_metadata_for_style(text_style));
  const float font_size_value = text_style.font_size;
  const TextSelectionRange selection = model_->selection();
  if (!selection.collapsed) {
    paint_text_selection_ranges(
        paint_list,
        *bounds,
        Color{.r = 0.22F, .g = 0.42F, .b = 0.80F, .a = 0.38F},
        text(),
        text_style.font,
        font_size_value,
        paint_list.scale(),
        selection);
  }
  if (!text().empty()) {
    paint_list.fill_text(
        *bounds,
        text_color,
        text(),
        text_style.font,
        font_size_value);
  }
  paint_text_caret_geometry(
      paint_list,
      *bounds,
      text_color,
      text(),
      text_style.font,
      font_size_value,
      paint_list.scale(),
      model_->cursor());
  paint_list.pop_metadata();
}

} // namespace cgpui
