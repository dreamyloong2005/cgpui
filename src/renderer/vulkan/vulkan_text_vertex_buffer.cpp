#include "vulkan_text_vertex_buffer_internal.hpp"

#include "vulkan_composition_transform_internal.hpp"
#include "vulkan_device_internal.hpp"

#include <cstring>

namespace cgpui {
namespace {

VulkanTextVertex make_text_vertex(
    float x,
    float y,
    float atlas_u,
    float atlas_v,
    const std::array<float, 4>& color) {
  return VulkanTextVertex{
      .position = {x, y},
      .atlas_uv = {atlas_u, atlas_v},
      .color = color,
  };
}

void append_text_quad_vertices(
    std::vector<VulkanTextVertex>& vertices,
    const TexturedGlyphQuad& quad,
    VulkanTextPositioningPolicy positioning_policy) {
  const Rect device_bounds =
      vulkan_position_text_bounds(quad.device_bounds, positioning_policy);
  const float left = device_bounds.origin.x;
  const float top = device_bounds.origin.y;
  const float right = left + device_bounds.size.width;
  const float bottom = top + device_bounds.size.height;
  const float atlas_left = quad.atlas_uv_bounds.origin.x;
  const float atlas_top = quad.atlas_uv_bounds.origin.y;
  const float atlas_right = atlas_left + quad.atlas_uv_bounds.size.width;
  const float atlas_bottom = atlas_top + quad.atlas_uv_bounds.size.height;
  const std::array<float, 4> color{
      quad.color.r,
      quad.color.g,
      quad.color.b,
      quad.color.a,
  };

  const Point top_left_point = vulkan_apply_composed_transform(
      Point{.x = left, .y = top}, quad.metadata, quad.composition_stack);
  const Point top_right_point = vulkan_apply_composed_transform(
      Point{.x = right, .y = top}, quad.metadata, quad.composition_stack);
  const Point bottom_right_point = vulkan_apply_composed_transform(
      Point{.x = right, .y = bottom}, quad.metadata, quad.composition_stack);
  const Point bottom_left_point = vulkan_apply_composed_transform(
      Point{.x = left, .y = bottom}, quad.metadata, quad.composition_stack);
  const VulkanTextVertex top_left = make_text_vertex(
      top_left_point.x, top_left_point.y, atlas_left, atlas_top, color);
  const VulkanTextVertex top_right = make_text_vertex(
      top_right_point.x, top_right_point.y, atlas_right, atlas_top, color);
  const VulkanTextVertex bottom_right = make_text_vertex(
      bottom_right_point.x,
      bottom_right_point.y,
      atlas_right,
      atlas_bottom,
      color);
  const VulkanTextVertex bottom_left = make_text_vertex(
      bottom_left_point.x,
      bottom_left_point.y,
      atlas_left,
      atlas_bottom,
      color);
  vertices.insert(
      vertices.end(),
      {top_left, top_right, bottom_right, top_left, bottom_right, bottom_left});
}

} // namespace

std::vector<VulkanTextVertex> vulkan_build_text_vertices(
    std::span<const TexturedGlyphQuad> quads,
    VulkanTextPositioningPolicy positioning_policy) {
  std::vector<VulkanTextVertex> vertices;
  vertices.reserve(quads.size() * vulkan_text_vertices_per_quad);
  for (const TexturedGlyphQuad& quad : quads) {
    append_text_quad_vertices(vertices, quad, positioning_policy);
  }
  return vertices;
}

VkBufferCreateInfo vulkan_text_vertex_buffer_create_info(
    std::size_t byte_size) {
  return VkBufferCreateInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = byte_size,
      .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
}

Result<void> vulkan_upload_text_vertex_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const TexturedGlyphQuad> quads,
    VulkanTextPositioningPolicy positioning_policy,
    VulkanTextVertexBufferResources& resources) {
  if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "text vertex upload requires Vulkan physical and logical devices"));
  }
  vulkan_destroy_text_vertex_buffer(device, resources);

  const std::vector<VulkanTextVertex> vertices =
      vulkan_build_text_vertices(quads, positioning_policy);
  resources.positioning_policy = positioning_policy;
  if (vertices.empty()) {
    return {};
  }
  const std::size_t byte_size = vertices.size() * sizeof(VulkanTextVertex);
  const VkBufferCreateInfo buffer_info =
      vulkan_text_vertex_buffer_create_info(byte_size);
  if (auto result = require_vk_success(
          vkCreateBuffer(device, &buffer_info, nullptr, &resources.buffer),
          "vkCreateBuffer for text vertices failed");
      !result) {
    return result;
  }

  VkMemoryRequirements requirements{};
  vkGetBufferMemoryRequirements(device, resources.buffer, &requirements);
  auto memory_type = vulkan_find_memory_type(
      physical_device,
      requirements.memoryTypeBits,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (!memory_type) {
    vulkan_destroy_text_vertex_buffer(device, resources);
    return std::unexpected(memory_type.error());
  }
  const VkMemoryAllocateInfo allocation_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = requirements.size,
      .memoryTypeIndex = *memory_type,
  };
  if (auto result = require_vk_success(
          vkAllocateMemory(device, &allocation_info, nullptr, &resources.memory),
          "vkAllocateMemory for text vertices failed");
      !result) {
    vulkan_destroy_text_vertex_buffer(device, resources);
    return result;
  }
  if (auto result = require_vk_success(
          vkBindBufferMemory(device, resources.buffer, resources.memory, 0),
          "vkBindBufferMemory for text vertices failed");
      !result) {
    vulkan_destroy_text_vertex_buffer(device, resources);
    return result;
  }

  void* mapped = nullptr;
  if (auto result = require_vk_success(
          vkMapMemory(device, resources.memory, 0, byte_size, 0, &mapped),
          "vkMapMemory for text vertices failed");
      !result) {
    vulkan_destroy_text_vertex_buffer(device, resources);
    return result;
  }
  std::memcpy(mapped, vertices.data(), byte_size);
  vkUnmapMemory(device, resources.memory);
  resources.vertex_count = vertices.size();
  resources.byte_size = byte_size;
  return {};
}

void vulkan_destroy_text_vertex_buffer(
    VkDevice device,
    VulkanTextVertexBufferResources& resources) {
  if (device == VK_NULL_HANDLE) {
    return;
  }
  if (resources.buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, resources.buffer, nullptr);
  }
  if (resources.memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, resources.memory, nullptr);
  }
  resources = {};
}

} // namespace cgpui
