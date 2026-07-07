#include "cgpui/ui/element_image_nodes.hpp"

#include <utility>

namespace cgpui {

ImageElement::ImageElement(
    ImageElementKind kind,
    ImageAssetDescriptor asset,
    StyleState style_state,
    std::optional<Rect> source_rect,
    std::optional<Color> tint,
    std::string alt)
    : kind_(kind),
      asset_(asset),
      style_state_(std::move(style_state)),
      source_rect_(source_rect),
      tint_(tint),
      alt_(std::move(alt)) {}

ImageElementKind ImageElement::kind() const {
  return kind_;
}

const ImageAssetDescriptor& ImageElement::asset() const {
  return asset_;
}

const StyleState& ImageElement::style_state() const {
  return style_state_;
}

const std::optional<Rect>& ImageElement::source_rect() const {
  return source_rect_;
}

const std::optional<Color>& ImageElement::tint() const {
  return tint_;
}

std::string_view ImageElement::alt() const {
  return alt_;
}

AccessibilityRole ImageElement::accessibility_role() const {
  return AccessibilityRole::image;
}

std::string ImageElement::accessibility_name() const {
  return alt_;
}

int ImageElement::z_index() const {
  return style_state_.base.z_index;
}

int ImageElement::layer() const {
  return style_state_.base.layer;
}

} // namespace cgpui
