#include "cgpui/ui/image_source.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

namespace cgpui {
namespace {

std::uint32_t pixel_extent(float value) {
  if (value <= 0.0F) {
    return 0;
  }
  const auto max_extent =
      static_cast<float>(std::numeric_limits<std::uint32_t>::max());
  return static_cast<std::uint32_t>(std::min(value, max_extent));
}

} // namespace

ImageSource::ImageSource(
    ImageSourceKind kind,
    ImageAssetDescriptor asset,
    std::string svg_source)
    : kind_(kind),
      asset_(asset),
      svg_source_(std::move(svg_source)) {}

ImageSourceKind ImageSource::kind() const {
  return kind_;
}

const ImageAssetDescriptor& ImageSource::asset() const {
  return asset_;
}

std::string_view ImageSource::svg_source() const {
  return svg_source_;
}

ImageSource image_source(ImageAssetDescriptor asset) {
  return ImageSource(ImageSourceKind::image, asset);
}

ImageSource image_source(const ImageAsset& asset) {
  return image_source(describe_image_asset(asset));
}

ImageSource svg_image_source(
    ImageAssetId id,
    Size logical_size,
    std::string_view svg_source) {
  const std::uint32_t pixel_width = pixel_extent(logical_size.width);
  const std::uint32_t pixel_height = pixel_extent(logical_size.height);
  return ImageSource(
      ImageSourceKind::svg,
      ImageAssetDescriptor{
          .id = id,
          .logical_size = logical_size,
          .pixel_width = pixel_width,
          .pixel_height = pixel_height,
          .stride = pixel_width * 4,
          .format = ImageFormat::rgba8_unorm,
          .byte_size = svg_source.size(),
      },
      std::string(svg_source));
}

} // namespace cgpui
