#include "cgpui/ui/element_slider_nodes.hpp"
#include <algorithm>

namespace cgpui {
namespace {

constexpr Size kDefaultSliderSize{.width = 120.0F, .height = 20.0F};
constexpr float kSliderThumbWidth = 8.0F;
constexpr float kSliderTrackHeight = 4.0F;

Size slider_preferred_size(const Style& style) {
  return Size{
      .width = style.preferred_size.width > 0.0F ? style.preferred_size.width
                                                 : kDefaultSliderSize.width,
      .height = style.preferred_size.height > 0.0F ? style.preferred_size.height
                                                   : kDefaultSliderSize.height,
  };
}

Rect slider_content_rect(std::optional<Rect> bounds, const Style& style) {
  if (!bounds.has_value()) {
    bounds = Rect{.size = slider_preferred_size(style)};
  }
  const float left = style.margin.left + style.padding.left;
  const float right = style.margin.right + style.padding.right;
  const float top = style.margin.top + style.padding.top;
  const float bottom = style.margin.bottom + style.padding.bottom;
  return Rect{
      .origin = {.x = bounds->origin.x + left, .y = bounds->origin.y + top},
      .size = {.width = std::max(0.0F, bounds->size.width - left - right),
               .height = std::max(0.0F, bounds->size.height - top - bottom)},
  };
}

} // namespace

LayoutOutput SliderElement::layout(LayoutInput input) const {
  const Style& style = style_state_.base;
  const Size content_size = slider_preferred_size(style);
  const Size preferred{
      .width = content_size.width + style.padding.left + style.padding.right +
               style.margin.left + style.margin.right,
      .height = content_size.height + style.padding.top + style.padding.bottom +
                style.margin.top + style.margin.bottom,
  };
  const LayoutOutput output{.size = constrain_size(preferred, input.constraints)};
  set_layout_bounds(Rect{.origin = output.origin, .size = output.size});
  return output;
}

Rect SliderElement::track_rect() const {
  const Rect content = slider_content_rect(layout_bounds(), style_state_.base);
  const float height = std::min(kSliderTrackHeight, content.size.height);
  return Rect{
      .origin = {.x = content.origin.x,
                 .y = content.origin.y + (content.size.height - height) / 2.0F},
      .size = {.width = content.size.width, .height = height},
  };
}

Rect SliderElement::thumb_rect() const {
  const Rect content = slider_content_rect(layout_bounds(), style_state_.base);
  const float width = std::min(kSliderThumbWidth, content.size.width);
  return Rect{
      .origin = {.x = content.origin.x +
                      normalized_value() *
                          std::max(0.0F, content.size.width - width),
                 .y = content.origin.y},
      .size = {.width = width, .height = content.size.height},
  };
}

} // namespace cgpui
