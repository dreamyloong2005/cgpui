#include "ui_internal.hpp"

#include "text_style_inheritance.hpp"

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
  const float glyph_width_value = glyph_width();
  const TextSelectionRange selection = model_->selection();
  if (!selection.collapsed) {
    paint_list.fill_text_selection(
        Rect{
            .origin =
                {
                    .x = bounds->origin.x +
                         (static_cast<float>(selection.start) *
                          glyph_width_value),
                    .y = bounds->origin.y,
                },
            .size =
                {
                    .width =
                        static_cast<float>(selection.end - selection.start) *
                        glyph_width_value,
                    .height = font_size_value,
                },
        },
        Color{.r = 0.22F, .g = 0.42F, .b = 0.80F, .a = 0.38F},
        selection,
        font_size_value);
  }
  if (!text().empty()) {
    paint_list.fill_text(
        *bounds,
        text_color,
        text(),
        text_style.font,
        font_size_value);
  }
  paint_list.fill_text_caret(
      Rect{
          .origin =
              {
                  .x = bounds->origin.x +
                       (static_cast<float>(model_->cursor()) *
                        glyph_width_value),
                  .y = bounds->origin.y,
              },
          .size = {.width = 1.0F, .height = font_size_value},
      },
      text_color,
      model_->cursor(),
      font_size_value);
  paint_list.pop_metadata();
}

} // namespace cgpui
