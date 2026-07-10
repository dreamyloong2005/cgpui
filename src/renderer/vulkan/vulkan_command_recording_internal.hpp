#pragma once

#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"
#include "vulkan_glyph_atlas_resources_internal.hpp"
#include "vulkan_glyph_atlas_uploads_internal.hpp"
#include "vulkan_frame_command_reuse_internal.hpp"
#include "vulkan_frame_draw_recording_internal.hpp"
#include "vulkan_frame_pixel_capture_internal.hpp"
#include "vulkan_image_texture_uploads_internal.hpp"

namespace cgpui {

Result<VulkanFrameCommandRecordingResult> record_vulkan_frame_command_buffer(
    VkCommandBuffer command_buffer,
    VkRenderPass render_pass,
    VkFramebuffer framebuffer,
    VkExtent2D extent,
    const VulkanRoundedRectPipelineResources& rounded_rect_pipeline_resources,
    const VulkanRoundedRectBufferResources& solid_rect_buffers,
    const VulkanRoundedRectBufferResources& rounded_rect_buffers,
    const VulkanTextPipelineResources& text_pipeline_resources,
    const VulkanTextVertexBufferResources& text_vertex_buffer,
    const VulkanImagePipelineResources& image_pipeline_resources,
    const VulkanImageVertexBufferResources& image_vertex_buffer,
    std::span<const ImageDraw> image_draws,
    Color color,
    std::span<const VulkanFrameDrawOrderEntry> draw_order,
    const VulkanGlyphAtlasResources& glyph_atlas_resources,
    std::span<const TexturedGlyphQuad> glyph_atlas_draw_quads,
    std::span<const VulkanGlyphAtlasDrawBinding> glyph_atlas_draw_bindings,
    const VulkanGlyphAtlasUploadResources& glyph_atlas_uploads,
    const VulkanImageTextureResources& image_texture_resources,
    const VulkanImageTextureUploadResources& image_texture_uploads,
    const VulkanFramePixelCaptureCommand& pixel_capture,
    VulkanFrameCommandReuseState& command_reuse_state);

} // namespace cgpui
