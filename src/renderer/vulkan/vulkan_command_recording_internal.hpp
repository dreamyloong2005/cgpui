#pragma once

#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"
#include "vulkan_glyph_atlas_resources_internal.hpp"
#include "vulkan_glyph_atlas_uploads_internal.hpp"
#include "vulkan_rounded_rect_draw_recording_internal.hpp"
#include "vulkan_solid_rect_recording_internal.hpp"
#include "vulkan_text_draw_recording_internal.hpp"

namespace cgpui {

Result<void> record_vulkan_frame_command_buffer(
    VkCommandBuffer command_buffer,
    VkRenderPass render_pass,
    VkFramebuffer framebuffer,
    VkExtent2D extent,
    const VulkanRoundedRectPipelineResources& rounded_rect_pipeline_resources,
    const VulkanRoundedRectBufferResources& rounded_rect_buffers,
    const VulkanTextPipelineResources& text_pipeline_resources,
    const VulkanTextVertexBufferResources& text_vertex_buffer,
    Color color,
    std::span<const SolidRect> rects,
    const VulkanGlyphAtlasResources& glyph_atlas_resources,
    std::span<const TexturedGlyphQuad> glyph_atlas_draw_quads,
    std::span<const VulkanGlyphAtlasDrawBinding> glyph_atlas_draw_bindings,
    const VulkanGlyphAtlasUploadResources& glyph_atlas_uploads);

} // namespace cgpui
