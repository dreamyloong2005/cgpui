#pragma once

#include "cgpui/renderer/renderer_types.hpp"

#include <string>
#include <string_view>

namespace cgpui {

enum class ImageSourceKind {
  image,
  svg,
};

class ImageSource {
 public:
  ImageSource() = default;
  ImageSource(
      ImageSourceKind kind,
      ImageAssetDescriptor asset,
      std::string svg_source = {});

  [[nodiscard]] ImageSourceKind kind() const;
  [[nodiscard]] const ImageAssetDescriptor& asset() const;
  [[nodiscard]] std::string_view svg_source() const;

 private:
  ImageSourceKind kind_ = ImageSourceKind::image;
  ImageAssetDescriptor asset_;
  std::string svg_source_;
};

[[nodiscard]] ImageSource image_source(ImageAssetDescriptor asset);
[[nodiscard]] ImageSource image_source(const ImageAsset& asset);
[[nodiscard]] ImageSource svg_image_source(
    ImageAssetId id,
    Size logical_size,
    std::string_view svg_source);

} // namespace cgpui
