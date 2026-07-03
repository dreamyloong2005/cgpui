#include "vulkan_report_internal.hpp"

#include <cstddef>

namespace cgpui {
namespace {

[[nodiscard]] Size atlas_page_size_for(
    const GlyphCache& glyph_cache,
    std::size_t page_index) {
  for (const GlyphAtlasPage& page : glyph_cache.atlas_pages()) {
    if (page.page_index == page_index) {
      return page.size;
    }
  }
  return Size{.width = 256.0F, .height = 256.0F};
}

[[nodiscard]] Rect normalized_atlas_bounds(Rect atlas_bounds, Size page_size) {
  const float width = page_size.width > 0.0F ? page_size.width : 1.0F;
  const float height = page_size.height > 0.0F ? page_size.height : 1.0F;
  return Rect{
      .origin =
          Point{
              .x = atlas_bounds.origin.x / width,
              .y = atlas_bounds.origin.y / height,
          },
      .size =
          Size{
              .width = atlas_bounds.size.width / width,
              .height = atlas_bounds.size.height / height,
          },
  };
}

} // namespace

std::vector<TexturedGlyphQuad> vulkan_build_textured_glyph_quads(
    const TextDraw& text,
    GlyphCache& glyph_cache) {
  std::vector<TexturedGlyphQuad> quads;
  quads.reserve(text.glyphs.size());

  for (const TextGlyphPaint& glyph : text.glyphs) {
    const GlyphCacheLookup lookup = glyph_cache.lookup(glyph.key);
    GlyphAtlasEntry entry;
    if (lookup.hit && lookup.entry.has_value()) {
      entry = *lookup.entry;
    } else {
      const GlyphAtlasAllocation allocation =
          glyph_cache.allocate(rasterize_fallback_glyph(glyph));
      entry = GlyphAtlasEntry{
          .key = allocation.key,
          .page_index = allocation.page_index,
          .atlas_bounds = allocation.atlas_bounds,
          .advance = glyph.device_advance,
      };
    }

    quads.push_back(TexturedGlyphQuad{
        .key = entry.key,
        .page_index = entry.page_index,
        .device_bounds =
            Rect{
                .origin = glyph.device_origin,
                .size = entry.atlas_bounds.size,
            },
        .atlas_bounds = entry.atlas_bounds,
        .atlas_uv_bounds = normalized_atlas_bounds(
            entry.atlas_bounds,
            atlas_page_size_for(glyph_cache, entry.page_index)),
        .color = text.color,
        .clip_rect = text.clip_rect,
        .clip_stack = text.clip_stack,
        .composition_stack = text.composition_stack,
        .metadata = text.metadata,
    });
  }

  return quads;
}

} // namespace cgpui
