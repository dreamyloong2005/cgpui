#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

void append_page_usage(
    std::vector<VulkanGlyphAtlasDrawPageUsage>& usages,
    std::size_t text_draw_index,
    std::size_t page_index) {
  const auto existing = std::ranges::find_if(
      usages,
      [&](const VulkanGlyphAtlasDrawPageUsage& usage) {
        return usage.text_draw_index == text_draw_index &&
               usage.page_index == page_index;
      });
  if (existing != usages.end()) {
    existing->glyph_quad_count += 1;
    return;
  }
  usages.push_back(VulkanGlyphAtlasDrawPageUsage{
      .text_draw_index = text_draw_index,
      .page_index = page_index,
      .glyph_quad_count = 1,
  });
}

[[nodiscard]] const VulkanGlyphAtlasPageResource* find_page_resource(
    const VulkanGlyphAtlasResources& resources,
    std::size_t page_index) {
  const auto page = std::ranges::find_if(
      resources.pages,
      [&](const VulkanGlyphAtlasPageResource& candidate) {
        return candidate.page_index == page_index;
      });
  return page == resources.pages.end() ? nullptr : &*page;
}

} // namespace

std::vector<VulkanGlyphAtlasDrawPageUsage>
vulkan_plan_glyph_atlas_draw_page_usages(
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  std::vector<VulkanGlyphAtlasDrawPageUsage> usages;
  for (std::size_t text_draw_index = 0;
       text_draw_index < text_draws.size();
       ++text_draw_index) {
    const std::vector<TexturedGlyphQuad> quads =
        vulkan_build_textured_glyph_quads(
            text_draws[text_draw_index],
            glyph_cache);
    for (const TexturedGlyphQuad& quad : quads) {
      append_page_usage(usages, text_draw_index, quad.page_index);
    }
  }
  return usages;
}

Result<std::vector<VulkanGlyphAtlasDrawBinding>>
vulkan_resolve_glyph_atlas_draw_bindings(
    std::span<const VulkanGlyphAtlasDrawPageUsage> usages,
    const VulkanGlyphAtlasResources& resources) {
  std::vector<VulkanGlyphAtlasDrawBinding> bindings;
  bindings.reserve(usages.size());
  for (const VulkanGlyphAtlasDrawPageUsage& usage : usages) {
    const VulkanGlyphAtlasPageResource* page =
        find_page_resource(resources, usage.page_index);
    if (page == nullptr || page->descriptor_set == VK_NULL_HANDLE) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "glyph atlas draw page descriptor is missing"));
    }
    bindings.push_back(VulkanGlyphAtlasDrawBinding{
        .text_draw_index = usage.text_draw_index,
        .page_index = usage.page_index,
        .glyph_quad_count = usage.glyph_quad_count,
        .descriptor_set = page->descriptor_set,
    });
  }
  return bindings;
}

Result<void> vulkan_validate_glyph_atlas_draw_bindings(
    std::span<const VulkanGlyphAtlasDrawBinding> bindings,
    const VulkanGlyphAtlasResources& resources) {
  for (const VulkanGlyphAtlasDrawBinding& binding : bindings) {
    const VulkanGlyphAtlasPageResource* page =
        find_page_resource(resources, binding.page_index);
    if (page == nullptr || binding.descriptor_set == VK_NULL_HANDLE ||
        page->descriptor_set != binding.descriptor_set) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "glyph atlas draw binding is stale"));
    }
  }
  return {};
}

} // namespace cgpui
