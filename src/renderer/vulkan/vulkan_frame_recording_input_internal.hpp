#pragma once

#include "vulkan_frame_effects_internal.hpp"
#include "vulkan_frame_pixel_capture_internal.hpp"
#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"
#include "vulkan_glyph_atlas_uploads_internal.hpp"
#include "vulkan_image_pipeline_internal.hpp"
#include "vulkan_image_texture_uploads_internal.hpp"
#include "vulkan_image_vertex_buffer_internal.hpp"
#include "vulkan_text_pipeline_resources_internal.hpp"
#include "vulkan_text_vertex_buffer_internal.hpp"

#include <span>

namespace cgpui {

struct VulkanFrameRecordingTarget {
  VkCommandBuffer command_buffer;
  VkRenderPass render_pass;
  VkFramebuffer framebuffer;
  VkExtent2D extent;
  Color clear_color;
};

struct VulkanFrameGeometryResources {
  const VulkanRoundedRectPipelineResources& pipeline;
  const VulkanRoundedRectBufferResources& solid_rect_buffers;
  const VulkanRoundedRectBufferResources& rounded_rect_buffers;
};

struct VulkanFrameTextResources {
  const VulkanTextPipelineResources& pipeline;
  const VulkanTextVertexBufferResources& vertex_buffer;
  const VulkanGlyphAtlasResources& glyph_atlas;
  std::span<const TexturedGlyphQuad> glyph_atlas_draw_quads;
  std::span<const VulkanGlyphAtlasDrawBinding> glyph_atlas_draw_bindings;
  const VulkanGlyphAtlasUploadResources& uploads;
};

struct VulkanFrameImageResources {
  const VulkanImagePipelineResources& pipeline;
  const VulkanImageVertexBufferResources& vertex_buffer;
  std::span<const ImageDraw> draws;
  const VulkanImageTextureResources& textures;
  const VulkanImageTextureUploadResources& uploads;
};

// Borrowed inputs for one synchronous recording call. The caller retains all
// resources; command reuse captures its own signature rather than these views.
struct VulkanFrameRecordingInput {
  VulkanFrameRecordingTarget target;
  VulkanFrameGeometryResources geometry;
  VulkanFrameTextResources text;
  VulkanFrameImageResources images;
  std::span<const VulkanFrameDrawOrderEntry> draw_order;
  const VulkanFramePixelCaptureCommand& pixel_capture;
  VulkanFrameEffects effects;
};

} // namespace cgpui
