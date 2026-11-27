#include "cgpui/renderer/glyph_cache.hpp"

#include <algorithm>

namespace cgpui {

GlyphAtlasPage GlyphCache::make_page(
    std::size_t page_index,
    Size minimum_size) {
  return GlyphAtlasPage{
      .page_index = page_index,
      .size =
          Size{
              .width = std::max(256.0F, minimum_size.width),
              .height = std::max(256.0F, minimum_size.height),
          },
  };
}

bool GlyphCache::allocate_from_page(
    GlyphAtlasPage& page,
    Size glyph_size,
    Rect& atlas_bounds) {
  if (glyph_size.width <= 0.0F || glyph_size.height <= 0.0F ||
      glyph_size.width > page.size.width ||
      glyph_size.height > page.size.height) {
    return false;
  }

  if (page.cursor.x + glyph_size.width > page.size.width) {
    page.cursor.x = 0.0F;
    page.cursor.y += page.row_height;
    page.row_height = 0.0F;
  }
  if (page.cursor.y + glyph_size.height > page.size.height) {
    return false;
  }

  atlas_bounds = Rect{.origin = page.cursor, .size = glyph_size};
  page.cursor.x += glyph_size.width;
  page.row_height = std::max(page.row_height, glyph_size.height);
  return true;
}

} // namespace cgpui
