#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

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
        .first_quad_index = usage.first_quad_index,
        .glyph_quad_count = usage.glyph_quad_count,
        .descriptor_set = page->descriptor_set,
        .clip_rect = usage.clip_rect,
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

Result<void> vulkan_validate_glyph_atlas_draw_bindings(
    std::span<const VulkanGlyphAtlasDrawBinding> bindings,
    std::span<const TexturedGlyphQuad> quads,
    const VulkanGlyphAtlasResources& resources) {
  if (auto result =
          vulkan_validate_glyph_atlas_draw_bindings(bindings, resources);
      !result) {
    return result;
  }
  for (const VulkanGlyphAtlasDrawBinding& binding : bindings) {
    if (binding.first_quad_index > quads.size() ||
        binding.glyph_quad_count > quads.size() - binding.first_quad_index) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "glyph atlas draw binding range is invalid"));
    }
    for (std::size_t index = 0; index < binding.glyph_quad_count; ++index) {
      if (quads[binding.first_quad_index + index].page_index !=
          binding.page_index) {
        return std::unexpected(vulkan_error(
            ErrorCode::renderer_initialization_failed,
            "glyph atlas draw binding page does not match quad data"));
      }
    }
  }
  return {};
}

} // namespace cgpui
