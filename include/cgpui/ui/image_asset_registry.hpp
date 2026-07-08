#pragma once

#include "cgpui/ui/image_source.hpp"

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace cgpui {

class RegisteredImageAsset {
 public:
  RegisteredImageAsset() = default;
  explicit RegisteredImageAsset(ImageSource source);

  [[nodiscard]] ImageAssetId id() const;
  [[nodiscard]] ImageSourceKind kind() const;
  [[nodiscard]] const ImageSource& source() const;
  [[nodiscard]] const ImageAssetDescriptor& asset() const;
  [[nodiscard]] std::string_view svg_source() const;

 private:
  ImageSource source_;
};

class ImageAssetRegistry {
 public:
  explicit ImageAssetRegistry(ImageAssetId first_id = ImageAssetId{1});

  [[nodiscard]] RegisteredImageAsset register_image(ImageAsset asset);
  [[nodiscard]] RegisteredImageAsset register_image(ImageAssetDescriptor asset);
  [[nodiscard]] RegisteredImageAsset register_svg(
      Size logical_size,
      std::string_view svg_source);

  [[nodiscard]] const RegisteredImageAsset* find(ImageAssetId id) const;
  [[nodiscard]] std::span<const RegisteredImageAsset> registrations() const;
  [[nodiscard]] std::span<const ImageAsset> raster_assets() const;

 private:
  [[nodiscard]] ImageAssetId allocate_id();
  void observe_id(ImageAssetId id);
  [[nodiscard]] RegisteredImageAsset store(ImageSource source);

  std::uint64_t next_id_;
  std::vector<RegisteredImageAsset> registrations_;
  std::vector<ImageAsset> raster_assets_;
};

} // namespace cgpui
