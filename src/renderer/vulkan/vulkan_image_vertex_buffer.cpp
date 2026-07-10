#include "vulkan_image_vertex_buffer_internal.hpp"

#include "vulkan_composition_transform_internal.hpp"
#include "vulkan_image_color_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

VulkanImageVertex make_image_vertex(
    Point position,
    float image_u,
    float image_v,
    Color color) {
  return VulkanImageVertex{
      .position = {position.x, position.y},
      .image_uv = {image_u, image_v},
      .color = {color.r, color.g, color.b, color.a},
  };
}

std::array<float, 4> image_uv_bounds(const ImageDraw& draw) {
  if (!draw.source_rect.has_value() || draw.asset.pixel_width == 0 ||
      draw.asset.pixel_height == 0) {
    return {0.0F, 0.0F, 1.0F, 1.0F};
  }
  const float pixel_width = static_cast<float>(draw.asset.pixel_width);
  const float pixel_height = static_cast<float>(draw.asset.pixel_height);
  const Rect source = *draw.source_rect;
  return {
      std::clamp(source.origin.x / pixel_width, 0.0F, 1.0F),
      std::clamp(source.origin.y / pixel_height, 0.0F, 1.0F),
      std::clamp(
          (source.origin.x + source.size.width) / pixel_width, 0.0F, 1.0F),
      std::clamp(
          (source.origin.y + source.size.height) / pixel_height, 0.0F, 1.0F),
  };
}

void append_image_quad_vertices(
    std::vector<VulkanImageVertex>& vertices,
    const ImageDraw& draw) {
  const float left = draw.bounds.origin.x;
  const float top = draw.bounds.origin.y;
  const float right = left + draw.bounds.size.width;
  const float bottom = top + draw.bounds.size.height;
  const auto uv = image_uv_bounds(draw);
  const Color color = vulkan_resolve_image_tint(draw);
  const Point top_left_point = vulkan_apply_composed_transform(
      Point{.x = left, .y = top}, draw.metadata, draw.composition_stack);
  const Point top_right_point = vulkan_apply_composed_transform(
      Point{.x = right, .y = top}, draw.metadata, draw.composition_stack);
  const Point bottom_right_point = vulkan_apply_composed_transform(
      Point{.x = right, .y = bottom}, draw.metadata, draw.composition_stack);
  const Point bottom_left_point = vulkan_apply_composed_transform(
      Point{.x = left, .y = bottom}, draw.metadata, draw.composition_stack);
  const VulkanImageVertex top_left =
      make_image_vertex(top_left_point, uv[0], uv[1], color);
  const VulkanImageVertex top_right =
      make_image_vertex(top_right_point, uv[2], uv[1], color);
  const VulkanImageVertex bottom_right =
      make_image_vertex(bottom_right_point, uv[2], uv[3], color);
  const VulkanImageVertex bottom_left =
      make_image_vertex(bottom_left_point, uv[0], uv[3], color);
  vertices.insert(
      vertices.end(),
      {top_left, top_right, bottom_right, top_left, bottom_right, bottom_left});
}

} // namespace

std::vector<VulkanImageVertex> vulkan_build_image_vertices(
    std::span<const ImageDraw> image_draws) {
  std::vector<VulkanImageVertex> vertices;
  vertices.reserve(image_draws.size() * vulkan_image_vertices_per_quad);
  for (const ImageDraw& draw : image_draws) {
    append_image_quad_vertices(vertices, draw);
  }
  return vertices;
}

Result<void> vulkan_upload_image_vertex_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const ImageDraw> image_draws,
    VulkanImageVertexBufferResources& resources) {
  if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image vertex upload requires Vulkan physical and logical devices"));
  }
  const std::vector<VulkanImageVertex> vertices =
      vulkan_build_image_vertices(image_draws);
  const std::size_t byte_size = vertices.size() * sizeof(VulkanImageVertex);
  if (auto result = vulkan_upload_frame_geometry_buffer(
      physical_device,
      device,
      vertices.data(),
      byte_size,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      resources.vertices);
      !result) {
    return result;
  }
  resources.vertex_count = vertices.size();
  resources.byte_size = byte_size;
  return {};
}

void vulkan_destroy_image_vertex_buffer(
    VkDevice device,
    VulkanImageVertexBufferResources& resources) {
  if (device == VK_NULL_HANDLE) {
    return;
  }
  vulkan_destroy_frame_geometry_buffer(device, resources.vertices);
  resources = {};
}

} // namespace cgpui
