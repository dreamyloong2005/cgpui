#pragma once

#include "vulkan_frame_draw_order_internal.hpp"

#include <optional>

namespace cgpui {

enum class VulkanFramePipelineKind {
  rounded_rect,
  text,
  image,
};

struct VulkanFramePipelineSwitchPlan {
  VulkanFrameDrawResourceKind resource_kind =
      VulkanFrameDrawResourceKind::solid_rect;
  VulkanFramePipelineKind pipeline_kind =
      VulkanFramePipelineKind::rounded_rect;
  bool bind_pipeline = true;
  bool bind_geometry = true;
};

struct VulkanFramePipelineSwitchState {
  std::optional<VulkanFramePipelineKind> active_pipeline;
  std::optional<VulkanFrameDrawResourceKind> active_resource;
};

[[nodiscard]] VulkanFramePipelineSwitchPlan vulkan_plan_frame_pipeline_switch(
    VulkanFramePipelineSwitchState& state,
    VulkanFrameDrawResourceKind resource_kind);

} // namespace cgpui
