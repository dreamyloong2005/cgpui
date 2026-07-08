#include "cgpui/ui/image_builder.hpp"

#include <utility>

namespace cgpui {

ImageBuilder::ImageBuilder(ImageElementKind kind, ImageAssetDescriptor asset)
    : ImageBuilder(kind, image_source(asset)) {}

ImageBuilder::ImageBuilder(ImageElementKind kind, ImageSource source)
    : kind_(kind),
      source_(std::move(source)) {}

ImageBuilder ImageBuilder::style(Style style) && {
  style_state_.base = std::move(style);
  return std::move(*this);
}

ImageBuilder ImageBuilder::size(Size size) && {
  style_state_.base.preferred_size = size;
  return std::move(*this);
}

ImageBuilder ImageBuilder::size(float side) && {
  return std::move(*this).size(Size{.width = side, .height = side});
}

ImageBuilder ImageBuilder::source_rect(Rect rect) && {
  source_rect_ = rect;
  return std::move(*this);
}

ImageBuilder ImageBuilder::tint(Color color) && {
  tint_ = color;
  return std::move(*this);
}

ImageBuilder ImageBuilder::alt(std::string_view text) && {
  alt_ = std::string(text);
  return std::move(*this);
}

ImageBuilder ImageBuilder::enabled(bool value) && {
  enabled_ = value;
  return std::move(*this);
}

ImageBuilder ImageBuilder::disabled() && {
  enabled_ = false;
  return std::move(*this);
}

ImageBuilder ImageBuilder::key(ElementKey key) && {
  key_ = std::move(key);
  return std::move(*this);
}

ImageBuilder ImageBuilder::key(std::string_view value) && {
  key_ = ElementKey{.value = std::string(value)};
  return std::move(*this);
}

AnyElement ImageBuilder::build() && {
  auto element = std::make_unique<ImageElement>(
      kind_,
      std::move(source_),
      style_state_,
      source_rect_,
      tint_,
      std::move(alt_));
  element->set_enabled(enabled_);
  element->set_key(key_);
  element->set_flex_grow(element->style_state().base.flex_grow);
  element->set_flex_shrink(element->style_state().base.flex_shrink);
  element->set_position(element->style_state().base.position);
  element->set_inset(element->style_state().base.inset);
  element->set_z_index(element->style_state().base.z_index);
  element->set_layer(element->style_state().base.layer);
  return element;
}

ImageBuilder image(ImageSource source) {
  return ImageBuilder(ImageElementKind::image, std::move(source));
}

ImageBuilder image(ImageAssetDescriptor asset) {
  return image(image_source(asset));
}

ImageBuilder image(const ImageAsset& asset) {
  return image(image_source(asset));
}

ImageBuilder icon(ImageSource source) {
  return ImageBuilder(ImageElementKind::icon, std::move(source));
}

ImageBuilder icon(ImageAssetDescriptor asset) {
  return icon(image_source(asset));
}

ImageBuilder icon(const ImageAsset& asset) {
  return icon(image_source(asset));
}

ImageBuilder svg(ImageSource source) {
  return ImageBuilder(ImageElementKind::svg, std::move(source));
}

ImageBuilder svg(
    ImageAssetId id,
    Size logical_size,
    std::string_view svg_source) {
  return svg(svg_image_source(id, logical_size, svg_source));
}

} // namespace cgpui
