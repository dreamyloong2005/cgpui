#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/renderer/glyph_uploads.hpp"
#include "cgpui/renderer/renderer_commands.hpp"

#include <cstddef>
#include <optional>
#include <span>
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
  [[nodiscard]] GlyphCacheLookup lookup(const GlyphAtlasKey& key);

  [[nodiscard]] GlyphAtlasAllocation allocate(const RasterizedGlyph& glyph);

  void store(GlyphAtlasEntry entry);

  [[nodiscard]] std::size_t lookup_count() const;

  [[nodiscard]] std::span<const GlyphCacheRecord> lookups() const;

  [[nodiscard]] std::span<const GlyphAtlasEntry> entries() const;

  [[nodiscard]] std::span<const GlyphAtlasPage> atlas_pages() const;

  [[nodiscard]] std::span<const GlyphUploadRecord> upload_records() const;

  void set_rasterization_error(Error error);
  void clear_rasterization_error();
  [[nodiscard]] const std::optional<Error>& rasterization_error() const;

 private:
  [[nodiscard]] const GlyphAtlasEntry* find_entry(
      const GlyphAtlasKey& key) const;

  [[nodiscard]] static GlyphAtlasPage make_page(
      std::size_t page_index,
      Size minimum_size);

  [[nodiscard]] static bool allocate_from_page(
      GlyphAtlasPage& page,
      Size glyph_size,
      Rect& atlas_bounds);

  std::vector<GlyphAtlasEntry> entries_;
  std::vector<GlyphCacheRecord> lookups_;
  std::vector<GlyphAtlasPage> pages_;
  std::vector<GlyphUploadRecord> uploads_;
  std::optional<Error> rasterization_error_;
};

void vulkan_consume_text_draw(const TextDraw& text, GlyphCache& glyph_cache);
std::vector<TexturedGlyphQuad> vulkan_build_textured_glyph_quads(
    const TextDraw& text,
    GlyphCache& glyph_cache);

} // namespace cgpui
