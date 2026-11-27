#include "cgpui/renderer/glyph_cache.hpp"

namespace cgpui {

GlyphAtlasAllocation GlyphCache::allocate(const RasterizedGlyph& glyph) {
  if (const GlyphAtlasEntry* existing = find_entry(glyph.key);
      existing != nullptr) {
    return GlyphAtlasAllocation{
        .key = glyph.key,
        .page_index = existing->page_index,
        .atlas_bounds = existing->atlas_bounds,
    };
  }

  const Size glyph_size{
      .width = static_cast<float>(glyph.bitmap.width),
      .height = static_cast<float>(glyph.bitmap.height),
  };
  if (pages_.empty()) {
    pages_.push_back(make_page(0, glyph_size));
  }

  Rect atlas_bounds;
  if (!allocate_from_page(pages_.back(), glyph_size, atlas_bounds)) {
    pages_.push_back(make_page(pages_.size(), glyph_size));
    (void)allocate_from_page(pages_.back(), glyph_size, atlas_bounds);
  }

  GlyphAtlasEntry entry{
      .key = glyph.key,
      .page_index = pages_.back().page_index,
      .atlas_bounds = atlas_bounds,
      .advance = glyph.advance,
      .left_bearing = glyph.left_bearing,
      .top_bearing = glyph.top_bearing,
      .baseline = glyph.baseline,
  };
  pages_.back().entries.push_back(entry);
  entries_.push_back(entry);
  uploads_.push_back(GlyphUploadRecord{
      .key = glyph.key,
      .page_index = entry.page_index,
      .atlas_bounds = atlas_bounds,
      .width = glyph.bitmap.width,
      .height = glyph.bitmap.height,
      .stride = glyph.bitmap.stride,
      .alpha = glyph.bitmap.alpha,
  });

  return GlyphAtlasAllocation{
      .key = glyph.key,
      .page_index = entry.page_index,
      .atlas_bounds = atlas_bounds,
      .created = true,
  };
}

} // namespace cgpui
