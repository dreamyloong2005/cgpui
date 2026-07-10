#pragma once

#include "cgpui/renderer/glyph_cache.hpp"
#include "vulkan_glyph_atlas_resources_internal.hpp"

#include <span>
#include <vector>

namespace cgpui {

struct VulkanGlyphAtlasDrawPageUsage {
  std::size_t text_draw_index = 0;
  std::size_t page_index = 0;
  std::size_t first_quad_index = 0;
  std::size_t glyph_quad_count = 0;
  std::optional<Rect> clip_rect;
};

struct VulkanGlyphAtlasDrawBinding {
  std::size_t text_draw_index = 0;
  std::size_t page_index = 0;
  std::size_t first_quad_index = 0;
  std::size_t glyph_quad_count = 0;
  VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
  std::optional<Rect> clip_rect;
};

struct VulkanGlyphAtlasDrawData {
  std::vector<TexturedGlyphQuad> quads;
  std::vector<VulkanGlyphAtlasDrawPageUsage> page_usages;
};

[[nodiscard]] VulkanGlyphAtlasDrawData vulkan_plan_glyph_atlas_draw_data(
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache);

[[nodiscard]] std::vector<VulkanGlyphAtlasDrawPageUsage>
vulkan_plan_glyph_atlas_draw_page_usages(
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache);

[[nodiscard]] Result<std::vector<VulkanGlyphAtlasDrawBinding>>
vulkan_resolve_glyph_atlas_draw_bindings(
    std::span<const VulkanGlyphAtlasDrawPageUsage> usages,
    const VulkanGlyphAtlasResources& resources);

Result<void> vulkan_validate_glyph_atlas_draw_bindings(
    std::span<const VulkanGlyphAtlasDrawBinding> bindings,
    const VulkanGlyphAtlasResources& resources);
Result<void> vulkan_validate_glyph_atlas_draw_bindings(
    std::span<const VulkanGlyphAtlasDrawBinding> bindings,
    std::span<const TexturedGlyphQuad> quads,
    const VulkanGlyphAtlasResources& resources);

} // namespace cgpui
