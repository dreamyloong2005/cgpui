#pragma once

#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"
#include "vulkan_glyph_atlas_resources_internal.hpp"
#include "vulkan_glyph_atlas_uploads_internal.hpp"

namespace cgpui {

Result<void> record_vulkan_frame_command_buffer(
    VkCommandBuffer command_buffer,
    VkRenderPass render_pass,
    VkFramebuffer framebuffer,
    VkExtent2D extent,
    Color color,
    std::span<const SolidRect> rects,
    const VulkanGlyphAtlasResources& glyph_atlas_resources,
    std::span<const VulkanGlyphAtlasDrawBinding> glyph_atlas_draw_bindings,
    const VulkanGlyphAtlasUploadResources& glyph_atlas_uploads);

} // namespace cgpui
