#include "vulkan_rounded_rect_buffers_internal.hpp"

#include "vulkan_solid_rect_geometry_internal.hpp"

namespace cgpui {
namespace {

Result<void> upload_geometry(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VulkanRoundedRectGeometry geometry,
    VulkanRoundedRectBufferResources& resources) {
  if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "rectangle upload requires Vulkan devices"));
  }
  const std::size_t vertex_byte_size =
      geometry.vertices.size() * sizeof(VulkanRoundedRectVertex);
  const std::size_t index_byte_size =
      geometry.indices.size() * sizeof(std::uint32_t);
  if (auto result = vulkan_upload_frame_geometry_buffer(
          physical_device,
          device,
          geometry.vertices.data(),
          vertex_byte_size,
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
          resources.vertices);
      !result) {
    return result;
  }
  if (auto result = vulkan_upload_frame_geometry_buffer(
          physical_device,
          device,
          geometry.indices.data(),
          index_byte_size,
          VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
          resources.indices);
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

} // namespace

Result<void> vulkan_upload_rounded_rect_buffers(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const RoundedRectDraw> rounded_rects,
    VulkanRoundedRectBufferResources& resources) {
  return upload_geometry(
      physical_device,
      device,
      vulkan_build_rounded_rect_geometry(rounded_rects),
      resources);
}

Result<void> vulkan_upload_solid_rect_buffers(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const SolidRect> rects,
    VulkanRoundedRectBufferResources& resources) {
  return upload_geometry(
      physical_device,
      device,
      vulkan_build_solid_rect_geometry(rects),
      resources);
}

void vulkan_destroy_rounded_rect_buffers(
    VkDevice device,
    VulkanRoundedRectBufferResources& resources) {
  if (device == VK_NULL_HANDLE) {
    return;
  }
  vulkan_destroy_frame_geometry_buffer(device, resources.indices);
  vulkan_destroy_frame_geometry_buffer(device, resources.vertices);
  resources = {};
}

} // namespace cgpui
