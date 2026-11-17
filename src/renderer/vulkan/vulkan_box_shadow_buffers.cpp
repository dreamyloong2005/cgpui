#include "vulkan_box_shadow_pipeline_internal.hpp"

#include <cstddef>

namespace cgpui {
namespace {

VkVertexInputAttributeDescription attribute(
    std::uint32_t location,
    VkFormat format,
    std::size_t offset) {
  return VkVertexInputAttributeDescription{
      .location = location,
      .binding = 0,
      .format = format,
      .offset = static_cast<std::uint32_t>(offset),
  };
}

} // namespace

VkVertexInputBindingDescription vulkan_box_shadow_vertex_binding_description() {
  return VkVertexInputBindingDescription{
      .binding = 0,
      .stride = sizeof(VulkanBoxShadowVertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
}

std::array<VkVertexInputAttributeDescription, 8>
vulkan_box_shadow_vertex_attribute_descriptions() {
  constexpr VkFormat vec2 = VK_FORMAT_R32G32_SFLOAT;
  constexpr VkFormat vec4 = VK_FORMAT_R32G32B32A32_SFLOAT;
  return {
      attribute(0, vec2, offsetof(VulkanBoxShadowVertex, position)),
      attribute(1, vec2, offsetof(VulkanBoxShadowVertex, local)),
      attribute(2, vec4, offsetof(VulkanBoxShadowVertex, color)),
      attribute(3, vec4, offsetof(VulkanBoxShadowVertex, shadow_rect)),
      attribute(4, vec4, offsetof(VulkanBoxShadowVertex, shadow_radii)),
      attribute(5, vec4, offsetof(VulkanBoxShadowVertex, box_rect)),
      attribute(6, vec4, offsetof(VulkanBoxShadowVertex, box_radii)),
      attribute(7, VK_FORMAT_R32_SFLOAT,
                offsetof(VulkanBoxShadowVertex, sigma)),
  };
}

Result<void> vulkan_upload_box_shadow_buffers(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const BoxShadowDraw> shadows,
    VulkanRoundedRectBufferResources& resources) {
  if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "box shadow upload requires Vulkan devices"));
  }
  VulkanBoxShadowGeometry geometry = vulkan_build_box_shadow_geometry(shadows);
  const std::size_t vertex_byte_size =
      geometry.vertices.size() * sizeof(VulkanBoxShadowVertex);
  const std::size_t index_byte_size =
      geometry.indices.size() * sizeof(std::uint32_t);
  if (auto result = vulkan_upload_frame_geometry_buffer(
          physical_device, device, geometry.vertices.data(), vertex_byte_size,
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, resources.vertices);
      !result) {
    return result;
  }
  if (auto result = vulkan_upload_frame_geometry_buffer(
          physical_device, device, geometry.indices.data(), index_byte_size,
          VK_BUFFER_USAGE_INDEX_BUFFER_BIT, resources.indices);
      !result) {
    return result;
  }
  resources.vertex_count = geometry.vertices.size();
  resources.index_count = geometry.indices.size();
  resources.vertex_byte_size = vertex_byte_size;
  resources.index_byte_size = index_byte_size;
  resources.draws = std::move(geometry.draws);
  return {};
}

} // namespace cgpui
