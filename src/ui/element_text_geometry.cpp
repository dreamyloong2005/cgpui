#include "cgpui/ui/element_text_nodes.hpp"

#include "text_composition_paint.hpp"
#include "text_selection_paint_geometry.hpp"

namespace cgpui {

std::string TextElement::display_text() const {
  if (model_ == nullptr || !model_->has_composition()) {
    return std::string(text());
  }
  return text_composition_display(*model_).text;
}

std::optional<Rect> TextElement::caret_rect(DpiScale scale) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || model_ == nullptr) {
    return {};
  }

  if (model_->has_composition()) {
    // IME candidates follow the composition caret, not the model cursor.
    const TextCompositionDisplay display = text_composition_display(*model_);
    return text_caret_rect(*bounds, display.text, effective_style_.font,
                           effective_style_.font_size, scale, display.caret);
  }
  return text_caret_rect(
      *bounds,
      text(),
      effective_style_.font,
      effective_style_.font_size,
      scale,
      model_->cursor());
}

std::optional<Rect> TextElement::composition_start_rect(
    DpiScale scale) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || model_ == nullptr || !model_->has_composition()) {
    return {};
  }
  const TextCompositionDisplay display = text_composition_display(*model_);
  return text_caret_rect(*bounds, display.text, effective_style_.font,
                         effective_style_.font_size, scale,
                         display.composition_start);
}

bool TextElement::scroll_caret_into_view(
    ScrollState& state,
    DpiScale scale) const {
  const std::optional<Rect> rect = caret_rect(scale);
  if (!rect.has_value()) {
    return false;
  }

  const Point before = state.offset();
  state.scroll_rect_into_view(*rect);
  const Point after = state.offset();
  return before.x != after.x || before.y != after.y;
}

} // namespace cgpui
