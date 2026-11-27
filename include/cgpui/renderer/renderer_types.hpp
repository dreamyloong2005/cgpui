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
  dx12,
};

[[nodiscard]] constexpr std::string_view renderer_backend_target_name(
    RendererBackendTarget target) {
  switch (target) {
    case RendererBackendTarget::vulkan:
      return "Vulkan";
    case RendererBackendTarget::metal:
      return "Metal";
    case RendererBackendTarget::dx12:
      return "DirectX 12";
  }

  return "Unknown";
}

[[nodiscard]] constexpr RendererBackendTarget default_renderer_backend_for(
    DesktopPlatformTarget target) {
  switch (target) {
    case DesktopPlatformTarget::windows:
      return RendererBackendTarget::dx12;
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
  bool transparent_background = false;
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

  [[nodiscard]] bool empty() const;

  friend bool operator==(
      const RendererClipStackRecord& lhs,
      const RendererClipStackRecord& rhs);
};

[[nodiscard]] RendererClipStackRecord renderer_clip_stack_record(
    std::span<const Rect> clips);

inline constexpr std::size_t kRendererCompositionStackMaxDepth = 8;

struct RendererCompositionStackRecord {
  std::vector<PaintMetadata> entries;
  std::size_t full_depth = 0;
  bool truncated = false;
  PaintMetadata current_metadata;

  [[nodiscard]] bool empty() const;

  friend bool operator==(
      const RendererCompositionStackRecord&,
      const RendererCompositionStackRecord&) = default;
};

[[nodiscard]] RendererCompositionStackRecord renderer_composition_stack_record(
    std::span<const PaintMetadata> entries);

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

[[nodiscard]] ImageAssetDescriptor describe_image_asset(
    const ImageAsset& asset);

} // namespace cgpui
