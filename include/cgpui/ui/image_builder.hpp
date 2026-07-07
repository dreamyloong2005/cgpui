#pragma once

#include "cgpui/renderer/renderer_types.hpp"
#include "cgpui/ui/element_builder_core.hpp"
#include "cgpui/ui/element_image_nodes.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace cgpui {

class ImageBuilder {
 public:
  ImageBuilder(ImageElementKind kind, ImageAssetDescriptor asset);

  [[nodiscard]] ImageBuilder style(Style style) &&;
  [[nodiscard]] ImageBuilder size(Size size) &&;
  [[nodiscard]] ImageBuilder size(float side) &&;
  [[nodiscard]] ImageBuilder source_rect(Rect rect) &&;
  [[nodiscard]] ImageBuilder tint(Color color) &&;
  [[nodiscard]] ImageBuilder alt(std::string_view text) &&;
  [[nodiscard]] ImageBuilder enabled(bool value) &&;
  [[nodiscard]] ImageBuilder disabled() &&;
  [[nodiscard]] ImageBuilder key(ElementKey key) &&;
  [[nodiscard]] ImageBuilder key(std::string_view value) &&;
  [[nodiscard]] AnyElement build() &&;

 private:
  ImageElementKind kind_;
  ImageAssetDescriptor asset_;
  StyleState style_state_;
  std::optional<Rect> source_rect_;
  std::optional<Color> tint_;
  std::string alt_;
  std::optional<ElementKey> key_;
  bool enabled_ = true;
};

[[nodiscard]] ImageBuilder image(ImageAssetDescriptor asset);
[[nodiscard]] ImageBuilder image(const ImageAsset& asset);
[[nodiscard]] ImageBuilder icon(ImageAssetDescriptor asset);
[[nodiscard]] ImageBuilder icon(const ImageAsset& asset);

} // namespace cgpui
