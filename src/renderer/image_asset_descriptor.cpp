#include "cgpui/renderer/renderer_types.hpp"

namespace cgpui {

ImageAssetDescriptor describe_image_asset(
    const ImageAsset& asset) {
  const Size logical_size =
      asset.logical_size.width > 0.0F && asset.logical_size.height > 0.0F
          ? asset.logical_size
          : Size{
                .width = static_cast<float>(asset.bitmap.width),
                .height = static_cast<float>(asset.bitmap.height),
            };
  return ImageAssetDescriptor{
      .id = asset.id,
      .logical_size = logical_size,
      .pixel_width = asset.bitmap.width,
      .pixel_height = asset.bitmap.height,
      .stride = asset.bitmap.stride,
      .format = asset.bitmap.format,
      .byte_size = asset.bitmap.pixels.size(),
  };
}

} // namespace cgpui
