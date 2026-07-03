#pragma once

#include "cgpui/renderer/glyph_uploads.hpp"
#include "cgpui/renderer/renderer_commands.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace cgpui {

struct GlyphCacheRecord {
  GlyphAtlasKey key;
  bool hit = false;
};

struct GlyphCacheLookup {
  GlyphAtlasKey key;
  bool hit = false;
  std::optional<GlyphAtlasEntry> entry;
};

class GlyphCache {
 public:
  [[nodiscard]] GlyphCacheLookup lookup(const GlyphAtlasKey& key) {
    for (const GlyphAtlasEntry& entry : entries_) {
      if (entry.key == key) {
        lookups_.push_back(GlyphCacheRecord{.key = key, .hit = true});
        return GlyphCacheLookup{.key = key, .hit = true, .entry = entry};
      }
    }

    lookups_.push_back(GlyphCacheRecord{.key = key, .hit = false});
    return GlyphCacheLookup{.key = key};
  }

  [[nodiscard]] GlyphAtlasAllocation allocate(const RasterizedGlyph& glyph) {
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

  void store(GlyphAtlasEntry entry) {
    for (GlyphAtlasEntry& existing : entries_) {
      if (existing.key == entry.key) {
        existing = std::move(entry);
        return;
      }
    }
    entries_.push_back(std::move(entry));
  }

  [[nodiscard]] std::size_t lookup_count() const {
    return lookups_.size();
  }

  [[nodiscard]] std::span<const GlyphCacheRecord> lookups() const {
    return lookups_;
  }

  [[nodiscard]] std::span<const GlyphAtlasEntry> entries() const {
    return entries_;
  }

  [[nodiscard]] std::span<const GlyphAtlasPage> atlas_pages() const {
    return pages_;
  }

  [[nodiscard]] std::span<const GlyphUploadRecord> upload_records() const {
    return uploads_;
  }

 private:
  [[nodiscard]] const GlyphAtlasEntry* find_entry(
      const GlyphAtlasKey& key) const {
    for (const GlyphAtlasEntry& entry : entries_) {
      if (entry.key == key) {
        return &entry;
      }
    }
    return nullptr;
  }

  [[nodiscard]] static GlyphAtlasPage make_page(
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

  [[nodiscard]] static bool allocate_from_page(
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

  std::vector<GlyphAtlasEntry> entries_;
  std::vector<GlyphCacheRecord> lookups_;
  std::vector<GlyphAtlasPage> pages_;
  std::vector<GlyphUploadRecord> uploads_;
};

void vulkan_consume_text_draw(const TextDraw& text, GlyphCache& glyph_cache);
std::vector<TexturedGlyphQuad> vulkan_build_textured_glyph_quads(
    const TextDraw& text,
    GlyphCache& glyph_cache);

} // namespace cgpui
