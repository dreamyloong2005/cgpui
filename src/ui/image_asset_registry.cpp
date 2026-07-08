#include "cgpui/ui/image_asset_registry.hpp"

#include <algorithm>
#include <utility>

namespace cgpui {

RegisteredImageAsset::RegisteredImageAsset(ImageSource source)
    : source_(std::move(source)) {}

ImageAssetId RegisteredImageAsset::id() const {
  return asset().id;
}

ImageSourceKind RegisteredImageAsset::kind() const {
  return source_.kind();
}

const ImageSource& RegisteredImageAsset::source() const {
  return source_;
}

const ImageAssetDescriptor& RegisteredImageAsset::asset() const {
  return source_.asset();
}

std::string_view RegisteredImageAsset::svg_source() const {
  return source_.svg_source();
}

ImageAssetRegistry::ImageAssetRegistry(ImageAssetId first_id)
    : next_id_(first_id.value == 0 ? 1 : first_id.value) {}

RegisteredImageAsset ImageAssetRegistry::register_image(ImageAsset asset) {
  if (asset.id.value == 0) {
    asset.id = allocate_id();
  } else {
    observe_id(asset.id);
  }

  RegisteredImageAsset registration = store(image_source(asset));
  raster_assets_.push_back(std::move(asset));
  return registration;
}

RegisteredImageAsset ImageAssetRegistry::register_image(
    ImageAssetDescriptor asset) {
  if (asset.id.value == 0) {
    asset.id = allocate_id();
  } else {
    observe_id(asset.id);
  }
  return store(image_source(asset));
}

RegisteredImageAsset ImageAssetRegistry::register_svg(
    Size logical_size,
    std::string_view svg_source) {
  return store(svg_image_source(allocate_id(), logical_size, svg_source));
}

const RegisteredImageAsset* ImageAssetRegistry::find(ImageAssetId id) const {
  const auto match = std::find_if(
      registrations_.begin(),
      registrations_.end(),
      [id](const RegisteredImageAsset& registration) {
        return registration.id() == id;
      });
  return match == registrations_.end() ? nullptr : &*match;
}

std::span<const RegisteredImageAsset> ImageAssetRegistry::registrations()
    const {
  return registrations_;
}

std::span<const ImageAsset> ImageAssetRegistry::raster_assets() const {
  return raster_assets_;
}

ImageAssetId ImageAssetRegistry::allocate_id() {
  return ImageAssetId{next_id_++};
}

void ImageAssetRegistry::observe_id(ImageAssetId id) {
  if (id.value >= next_id_) {
    next_id_ = id.value + 1;
  }
}

RegisteredImageAsset ImageAssetRegistry::store(ImageSource source) {
  registrations_.push_back(RegisteredImageAsset(std::move(source)));
  return registrations_.back();
}

} // namespace cgpui
