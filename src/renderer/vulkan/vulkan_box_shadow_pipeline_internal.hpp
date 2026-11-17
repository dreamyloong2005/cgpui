#pragma once

#include "vulkan_box_shadow_geometry_internal.hpp"
#include "vulkan_rounded_rect_buffers_internal.hpp"
#include "vulkan_rounded_rect_pipeline_internal.hpp"

#include <array>
#include <span>

namespace cgpui {

// The box shadow pipeline shares the rounded rect pipeline layout (an
// 8-byte framebuffer-size push constant), fixed state, blending, and draw
// recording; only the shaders and the vertex layout differ.
struct VulkanBoxShadowDrawResources {
  const VulkanRoundedRectPipelineResources* pipeline = nullptr;
  const VulkanRoundedRectBufferResources* buffers = nullptr;
};

[[nodiscard]] std::span<const std::uint32_t>
vulkan_box_shadow_vertex_shader_spirv();
[[nodiscard]] std::span<const std::uint32_t>
vulkan_box_shadow_fragment_shader_spirv();
[[nodiscard]] VkVertexInputBindingDescription
vulkan_box_shadow_vertex_binding_description();
[[nodiscard]] std::array<VkVertexInputAttributeDescription, 8>
vulkan_box_shadow_vertex_attribute_descriptions();
Result<void> vulkan_create_box_shadow_pipeline_resources(
    VkDevice device,
    VkRenderPass render_pass,
    VulkanRoundedRectPipelineResources& resources);
Result<void> vulkan_upload_box_shadow_buffers(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const BoxShadowDraw> shadows,
    VulkanRoundedRectBufferResources& resources);

} // namespace cgpui
