#include "cgpui/renderer/glyph_cache.hpp"

#include <utility>

namespace cgpui {

GlyphCacheLookup GlyphCache::lookup(const GlyphAtlasKey& key) {
  for (const GlyphAtlasEntry& entry : entries_) {
    if (entry.key == key) {
      lookups_.push_back(GlyphCacheRecord{.key = key, .hit = true});
      return GlyphCacheLookup{.key = key, .hit = true, .entry = entry};
    }
  }

  lookups_.push_back(GlyphCacheRecord{.key = key, .hit = false});
  return GlyphCacheLookup{.key = key};
}

void GlyphCache::store(GlyphAtlasEntry entry) {
  for (GlyphAtlasEntry& existing : entries_) {
    if (existing.key == entry.key) {
      existing = std::move(entry);
      return;
    }
  }
  entries_.push_back(std::move(entry));
}

std::size_t GlyphCache::lookup_count() const {
  return lookups_.size();
}

std::span<const GlyphCacheRecord> GlyphCache::lookups() const {
  return lookups_;
}

std::span<const GlyphAtlasEntry> GlyphCache::entries() const {
  return entries_;
}

std::span<const GlyphAtlasPage> GlyphCache::atlas_pages() const {
  return pages_;
}

std::span<const GlyphUploadRecord> GlyphCache::upload_records() const {
  return uploads_;
}

const GlyphAtlasEntry* GlyphCache::find_entry(
    const GlyphAtlasKey& key) const {
  for (const GlyphAtlasEntry& entry : entries_) {
    if (entry.key == key) {
      return &entry;
    }
  }
  return nullptr;
}

} // namespace cgpui
