#include "cgpui/ui/element_image_nodes.hpp"

#include <algorithm>

namespace cgpui {
namespace {

Size resolved_image_size(ImageElementKind kind, const Style& style, Size asset) {
  if (style.preferred_size.width > 0.0F && style.preferred_size.height > 0.0F) {
    return style.preferred_size;
  }
  if (kind == ImageElementKind::icon) {
    const float side =
        std::max({style.preferred_size.width, style.preferred_size.height, 16.0F});
    return Size{.width = side, .height = side};
  }
  return asset;
}

} // namespace

Size ImageElement::intrinsic_size() const {
  return Size{
      .width = asset_.logical_size.width > 0.0F
                   ? asset_.logical_size.width
                   : static_cast<float>(asset_.pixel_width),
      .height = asset_.logical_size.height > 0.0F
                    ? asset_.logical_size.height
                    : static_cast<float>(asset_.pixel_height),
  };
}

LayoutOutput ImageElement::layout(LayoutInput input) const {
  const Style& style = style_state_.base;
  const Size content_size = resolved_image_size(kind_, style, intrinsic_size());
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

Rect ImageElement::content_rect() const {
  const Style& style = style_state_.base;
  const Rect bounds =
      layout_bounds().value_or(Rect{.origin = {}, .size = intrinsic_size()});
  const float left = style.margin.left + style.padding.left;
  const float right = style.margin.right + style.padding.right;
  const float top = style.margin.top + style.padding.top;
  const float bottom = style.margin.bottom + style.padding.bottom;
  return Rect{
      .origin = {.x = bounds.origin.x + left, .y = bounds.origin.y + top},
      .size = {.width = std::max(0.0F, bounds.size.width - left - right),
               .height = std::max(0.0F, bounds.size.height - top - bottom)},
  };
}

} // namespace cgpui
