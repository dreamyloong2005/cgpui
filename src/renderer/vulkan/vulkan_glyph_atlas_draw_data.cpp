#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"

#include <utility>

namespace cgpui {
namespace {

void append_page_run(
    std::vector<VulkanGlyphAtlasDrawPageUsage>& usages,
    std::size_t text_draw_index,
    std::size_t page_index,
    std::size_t quad_index) {
  if (!usages.empty()) {
    VulkanGlyphAtlasDrawPageUsage& previous = usages.back();
    if (previous.text_draw_index == text_draw_index &&
        previous.page_index == page_index &&
        previous.first_quad_index + previous.glyph_quad_count == quad_index) {
      previous.glyph_quad_count += 1;
      return;
    }
  }
  usages.push_back(VulkanGlyphAtlasDrawPageUsage{
      .text_draw_index = text_draw_index,
      .page_index = page_index,
      .first_quad_index = quad_index,
      .glyph_quad_count = 1,
  });
}

} // namespace

VulkanGlyphAtlasDrawData vulkan_plan_glyph_atlas_draw_data(
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  VulkanGlyphAtlasDrawData draw_data;
  for (std::size_t text_draw_index = 0;
       text_draw_index < text_draws.size();
       ++text_draw_index) {
    std::vector<TexturedGlyphQuad> quads = vulkan_build_textured_glyph_quads(
        text_draws[text_draw_index],
        glyph_cache);
    for (TexturedGlyphQuad& quad : quads) {
      const std::size_t quad_index = draw_data.quads.size();
      append_page_run(
          draw_data.page_usages,
          text_draw_index,
          quad.page_index,
          quad_index);
      draw_data.quads.push_back(std::move(quad));
    }
  }
  return draw_data;
}

std::vector<VulkanGlyphAtlasDrawPageUsage>
vulkan_plan_glyph_atlas_draw_page_usages(
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  return vulkan_plan_glyph_atlas_draw_data(text_draws, glyph_cache).page_usages;
}

} // namespace cgpui
