#pragma once

#include "cgpui/renderer/glyph_atlas_types.hpp"
#include "vulkan_frame_geometry_buffer_internal.hpp"
#include "vulkan_text_pipeline_internal.hpp"
#include "vulkan_text_positioning_internal.hpp"

namespace cgpui {

inline constexpr std::size_t vulkan_text_vertices_per_quad = 6;

struct VulkanTextVertexBufferResources {
  VulkanFrameGeometryBufferResources vertices;
  std::size_t vertex_count = 0;
  std::size_t byte_size = 0;
  VulkanTextPositioningPolicy positioning_policy =
      VulkanTextPositioningPolicy::preserve_subpixel;

  [[nodiscard]] bool ready() const {
    return vertices.allocated() && vertex_count != 0 && byte_size != 0;
  }
};

[[nodiscard]] std::vector<VulkanTextVertex> vulkan_build_text_vertices(
    std::span<const TexturedGlyphQuad> quads,
    VulkanTextPositioningPolicy positioning_policy =
        VulkanTextPositioningPolicy::preserve_subpixel);
Result<void> vulkan_upload_text_vertex_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const TexturedGlyphQuad> quads,
    VulkanTextPositioningPolicy positioning_policy,
    VulkanTextVertexBufferResources& resources);
void vulkan_destroy_text_vertex_buffer(
    VkDevice device,
    VulkanTextVertexBufferResources& resources);

} // namespace cgpui
