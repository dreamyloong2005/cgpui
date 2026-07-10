#pragma once

#include "vulkan_rounded_rect_geometry_internal.hpp"
#include "vulkan_text_draw_recording_internal.hpp"

#include <optional>
#include <span>

namespace cgpui {

struct VulkanFrameDrawOrderEntry {
  RendererPrimitiveKind primitive_kind = RendererPrimitiveKind::solid_rect;
  std::size_t command_index = 0;

  friend bool operator==(
      const VulkanFrameDrawOrderEntry&,
      const VulkanFrameDrawOrderEntry&) = default;
};

enum class VulkanFrameDrawResourceKind {
  solid_rect,
  rounded_rect,
  text,
};

struct VulkanResolvedFrameDraw {
  VulkanFrameDrawResourceKind resource_kind =
      VulkanFrameDrawResourceKind::solid_rect;
  std::size_t resource_index = 0;

  friend bool operator==(
      const VulkanResolvedFrameDraw&,
      const VulkanResolvedFrameDraw&) = default;
};

class VulkanFrameDrawOrderCursor {
 public:
  VulkanFrameDrawOrderCursor(
      std::span<const VulkanFrameDrawOrderEntry> order,
      std::span<const VulkanRoundedRectDrawRange> solid_draws,
      std::span<const VulkanRoundedRectDrawRange> rounded_draws,
      std::span<const VulkanTextDrawCommand> text_commands);

  [[nodiscard]] std::optional<VulkanResolvedFrameDraw> next();

 private:
  std::span<const VulkanFrameDrawOrderEntry> order_;
  std::span<const VulkanRoundedRectDrawRange> solid_draws_;
  std::span<const VulkanRoundedRectDrawRange> rounded_draws_;
  std::span<const VulkanTextDrawCommand> text_commands_;
  std::size_t order_index_ = 0;
  std::size_t solid_index_ = 0;
  std::size_t rounded_index_ = 0;
  std::size_t text_index_ = 0;
};

} // namespace cgpui
