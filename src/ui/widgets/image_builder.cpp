#include "cgpui/ui/image_builder.hpp"

#include <utility>

namespace cgpui {

ImageBuilder::ImageBuilder(ImageElementKind kind, ImageAssetDescriptor asset)
    : kind_(kind),
      asset_(asset) {}

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
      asset_,
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

ImageBuilder image(ImageAssetDescriptor asset) {
  return ImageBuilder(ImageElementKind::image, asset);
}

ImageBuilder image(const ImageAsset& asset) {
  return image(describe_image_asset(asset));
}

ImageBuilder icon(ImageAssetDescriptor asset) {
  return ImageBuilder(ImageElementKind::icon, asset);
}

ImageBuilder icon(const ImageAsset& asset) {
  return icon(describe_image_asset(asset));
}

} // namespace cgpui
