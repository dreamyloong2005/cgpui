#pragma once

#include "dx12_texture_internal.hpp"
#include "dx12_glyph_lineage_internal.hpp"
#include "cgpui/renderer/glyph_cache.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace cgpui {
struct Dx12State;

struct Dx12GlyphPage {
  std::uint32_t width = 0, height = 0;
  std::vector<std::uint8_t> alpha;
  Dx12TextureHandle texture;
  bool dirty = true;
};

// CPU atlas storage survives frames. Each published texture is immutable so
// adding glyphs never changes an earlier draw's recorded resource snapshot.
struct Dx12TextCache {
  GlyphCache glyphs;
  Dx12GlyphLineage lineage;
  std::vector<Dx12GlyphPage> pages;
  std::size_t processed_uploads = 0;
};

Result<void> dx12_update_glyph_pages(Dx12State& state, Dx12TextCache& cache);

} // namespace cgpui
