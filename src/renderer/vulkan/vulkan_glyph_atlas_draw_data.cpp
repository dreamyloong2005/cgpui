#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"

#include "vulkan_clip_scissor_internal.hpp"

#include <utility>

namespace cgpui {
namespace {

void append_page_run(
    std::vector<VulkanGlyphAtlasDrawPageUsage>& usages,
    std::size_t text_draw_index,
    std::size_t page_index,
    std::size_t quad_index,
    std::optional<Rect> clip_rect) {
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
      .clip_rect = clip_rect,
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
    const TextDraw& text_draw = text_draws[text_draw_index];
    const std::optional<Rect> clip_rect = vulkan_resolve_effective_clip_rect(
        text_draw.clip_rect, text_draw.clip_stack);
    std::vector<TexturedGlyphQuad> quads = vulkan_build_textured_glyph_quads(
        text_draw,
        glyph_cache);
    for (TexturedGlyphQuad& quad : quads) {
      const std::size_t quad_index = draw_data.quads.size();
      append_page_run(
          draw_data.page_usages,
          text_draw_index,
          quad.page_index,
          quad_index,
          clip_rect);
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
