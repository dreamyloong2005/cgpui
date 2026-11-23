#include "dx12_image_cache_internal.hpp"

namespace cgpui {
Result<void> Dx12ImageCache::upload(Dx12State& state, const ImageAsset& image) {
  if (image.id.value == 0 || image.bitmap.format != ImageFormat::rgba8_unorm) {
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 image upload requires a nonzero asset id and RGBA8 pixels"});
  }
  auto texture = dx12_create_texture(state, image.bitmap.width, image.bitmap.height,
      image.bitmap.stride, DXGI_FORMAT_R8G8B8A8_UNORM, image.bitmap.pixels);
  if (!texture) return std::unexpected(texture.error());
  entries_.insert_or_assign(image.id.value, Entry{describe_image_asset(image), std::move(*texture)});
  return {};
}

void Dx12ImageCache::invalidate(ImageAssetId id) { entries_.erase(id.value); }

Result<Dx12TextureHandle> Dx12ImageCache::find(const ImageAssetDescriptor& descriptor) {
  const auto found = entries_.find(descriptor.id.value);
  if (found == entries_.end()) {
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 image draw requires a previously uploaded asset"});
  }
  auto& entry = found->second;
  if (entry.descriptor.pixel_width != descriptor.pixel_width ||
      entry.descriptor.pixel_height != descriptor.pixel_height ||
      entry.descriptor.format != descriptor.format) {
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 image draw descriptor does not match its uploaded asset"});
  }
  entry.touched = true;
  return entry.texture;
}

void Dx12ImageCache::finish_frame() {
  for (auto entry = entries_.begin(); entry != entries_.end();) {
    auto& value = entry->second;
    if (value.touched) value.idle_frames = 0;
    else ++value.idle_frames;
    value.touched = false;
    if (value.idle_frames > 120) entry = entries_.erase(entry);
    else ++entry;
  }
}
} // namespace cgpui
