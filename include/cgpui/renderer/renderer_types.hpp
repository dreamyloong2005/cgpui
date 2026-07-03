#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/native_surface.hpp"
#include "cgpui/platform/target.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace cgpui {

enum class RendererBackendTarget {
  vulkan,
  metal,
};

[[nodiscard]] constexpr std::string_view renderer_backend_target_name(
    RendererBackendTarget target) {
  switch (target) {
    case RendererBackendTarget::vulkan:
      return "Vulkan";
    case RendererBackendTarget::metal:
      return "Metal";
  }

  return "Unknown";
}

[[nodiscard]] constexpr RendererBackendTarget default_renderer_backend_for(
    DesktopPlatformTarget target) {
  switch (target) {
    case DesktopPlatformTarget::windows:
    case DesktopPlatformTarget::linux_wayland:
      return RendererBackendTarget::vulkan;
    case DesktopPlatformTarget::macos_cocoa:
      return RendererBackendTarget::metal;
  }

  return RendererBackendTarget::vulkan;
}

struct RenderSurfaceDescriptor {
  NativeSurfaceHandle native_surface;
  Size framebuffer_size;
  DpiScale scale;
};

struct PaintMetadata {
  float opacity = 1.0F;
  AffineTransform transform;

  friend bool operator==(
      const PaintMetadata&,
      const PaintMetadata&) = default;
};

inline constexpr std::size_t kRendererClipStackMaxDepth = 8;

struct RendererClipStackRecord {
  std::vector<Rect> clips;
  std::size_t full_depth = 0;
  bool truncated = false;
  std::optional<Rect> current_clip_rect;

  [[nodiscard]] bool empty() const { return full_depth == 0; }

  friend bool operator==(
      const RendererClipStackRecord& lhs,
      const RendererClipStackRecord& rhs) {
    const auto same_rect = [](Rect left, Rect right) {
      return left.origin.x == right.origin.x && left.origin.y == right.origin.y &&
             left.size.width == right.size.width &&
             left.size.height == right.size.height;
    };
    const auto same_optional_rect =
        [&](const std::optional<Rect>& left, const std::optional<Rect>& right) {
          if (left.has_value() != right.has_value()) {
            return false;
          }
          return !left.has_value() || same_rect(*left, *right);
        };
    if (lhs.full_depth != rhs.full_depth || lhs.truncated != rhs.truncated ||
        !same_optional_rect(lhs.current_clip_rect, rhs.current_clip_rect) ||
        lhs.clips.size() != rhs.clips.size()) {
      return false;
    }
    for (std::size_t index = 0; index < lhs.clips.size(); ++index) {
      if (!same_rect(lhs.clips[index], rhs.clips[index])) {
        return false;
      }
    }
    return true;
  }
};

[[nodiscard]] inline RendererClipStackRecord renderer_clip_stack_record(
    std::span<const Rect> clips) {
  RendererClipStackRecord record;
  record.full_depth = clips.size();
  if (clips.empty()) {
    return record;
  }

  record.current_clip_rect = clips.back();
  record.truncated = clips.size() > kRendererClipStackMaxDepth;
  const std::size_t first_kept =
      record.truncated ? clips.size() - kRendererClipStackMaxDepth : 0;
  record.clips.assign(clips.begin() + first_kept, clips.end());
  return record;
}

inline constexpr std::size_t kRendererCompositionStackMaxDepth = 8;

struct RendererCompositionStackRecord {
  std::vector<PaintMetadata> entries;
  std::size_t full_depth = 0;
  bool truncated = false;
  PaintMetadata current_metadata;

  [[nodiscard]] bool empty() const { return full_depth == 0; }

  friend bool operator==(
      const RendererCompositionStackRecord&,
      const RendererCompositionStackRecord&) = default;
};

[[nodiscard]] inline RendererCompositionStackRecord
renderer_composition_stack_record(std::span<const PaintMetadata> entries) {
  RendererCompositionStackRecord record;
  record.full_depth = entries.size();
  if (entries.empty()) {
    return record;
  }

  record.current_metadata = entries.back();
  record.truncated = entries.size() > kRendererCompositionStackMaxDepth;
  const std::size_t first_kept =
      record.truncated ? entries.size() - kRendererCompositionStackMaxDepth : 0;
  record.entries.assign(entries.begin() + first_kept, entries.end());
  return record;
}

enum class ImageFormat {
  rgba8_unorm,
};

struct DecodedImageBitmap {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t stride = 0;
  ImageFormat format = ImageFormat::rgba8_unorm;
  std::vector<std::uint8_t> pixels;
};

struct ImageAssetId {
  std::uint64_t value = 0;

  friend bool operator==(ImageAssetId, ImageAssetId) = default;
};

struct ImageAsset {
  ImageAssetId id;
  Size logical_size;
  DecodedImageBitmap bitmap;
};

struct ImageAssetDescriptor {
  ImageAssetId id;
  Size logical_size;
  std::uint32_t pixel_width = 0;
  std::uint32_t pixel_height = 0;
  std::uint32_t stride = 0;
  ImageFormat format = ImageFormat::rgba8_unorm;
  std::size_t byte_size = 0;

  friend bool operator==(
      const ImageAssetDescriptor&,
      const ImageAssetDescriptor&) = default;
};

[[nodiscard]] inline ImageAssetDescriptor describe_image_asset(
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
