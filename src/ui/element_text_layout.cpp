#include "cgpui/ui/element_text_nodes.hpp"

#include "element_stack_box_internal.hpp"

namespace cgpui {

namespace {

// Wraps `content` inside the style's content box and sizes the element as
// that box plus padding and margin, like v_stack / row / column.
Size layout_text_box(
    std::string_view content,
    const FontDescriptor& font,
    float font_size,
    const Style& box_style,
    const LayoutInput& input) {
  const TextMeasurement measurement =
      measure_text(content, font, font_size, input.scale);
  const TextWrapLayout wrap_layout = wrap_text_measurement(
      measurement,
      stack_box_content_max_width(input.constraints.max_size.width,
                                  box_style));
  return resolve_stack_box(wrap_layout.logical_size, box_style,
                           input.constraints)
      .outer;
}

std::optional<Rect> content_bounds(
    const std::optional<Rect>& bounds,
    const Style& box_style) {
  if (!bounds.has_value()) {
    return std::nullopt;
  }
  return stack_box_content_rect(*bounds, box_style);
}

} // namespace

LayoutOutput TextElement::layout(LayoutInput input) const {
  const LayoutOutput output{
      .size = layout_text_box(display_text(), font(), font_size(), style_,
                              input),
  };
  set_layout_bounds(Rect{.origin = output.origin, .size = output.size});
  return output;
}

std::optional<Rect> TextElement::text_bounds() const {
  return content_bounds(layout_bounds(), style_);
}

LayoutOutput LabelElement::layout(LayoutInput input) const {
  const LayoutOutput output{
      .size = layout_text_box(text_, font(), font_size(), style_, input),
  };
  set_layout_bounds(Rect{.origin = output.origin, .size = output.size});
  return output;
}

std::optional<Rect> LabelElement::text_bounds() const {
  return content_bounds(layout_bounds(), style_);
}

} // namespace cgpui
