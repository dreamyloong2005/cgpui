#include "vulkan_frame_pipeline_switch_internal.hpp"

namespace cgpui {
namespace {

VulkanFramePipelineKind pipeline_kind_for_resource(
    VulkanFrameDrawResourceKind resource_kind) {
  switch (resource_kind) {
    case VulkanFrameDrawResourceKind::solid_rect:
    case VulkanFrameDrawResourceKind::rounded_rect:
      return VulkanFramePipelineKind::rounded_rect;
    case VulkanFrameDrawResourceKind::text:
      return VulkanFramePipelineKind::text;
    case VulkanFrameDrawResourceKind::image:
      return VulkanFramePipelineKind::image;
  }
  return VulkanFramePipelineKind::rounded_rect;
}

} // namespace

VulkanFramePipelineSwitchPlan vulkan_plan_frame_pipeline_switch(
    VulkanFramePipelineSwitchState& state,
    VulkanFrameDrawResourceKind resource_kind) {
  const VulkanFramePipelineKind pipeline_kind =
      pipeline_kind_for_resource(resource_kind);
  const VulkanFramePipelineSwitchPlan plan{
      .resource_kind = resource_kind,
      .pipeline_kind = pipeline_kind,
      .bind_pipeline = state.active_pipeline != pipeline_kind,
      .bind_geometry = state.active_resource != resource_kind,
  };
  state.active_pipeline = pipeline_kind;
  state.active_resource = resource_kind;
  return plan;
}

} // namespace cgpui
