#pragma once

#include "cgpui/renderer/renderer_types.hpp"
#include "cgpui/ui/text.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace cgpui {

enum class GlyphAtlasImageFormat {
  alpha8_unorm,
};

struct GlyphAtlasEntry {
  GlyphAtlasKey key;
  std::size_t page_index = 0;
  Rect atlas_bounds;
  float advance = 0.0F;

  friend bool operator==(
      const GlyphAtlasEntry&,
      const GlyphAtlasEntry&) = default;
};

struct GlyphAtlasAllocation {
  GlyphAtlasKey key;
  std::size_t page_index = 0;
  Rect atlas_bounds;
  bool created = false;
};

struct GlyphAtlasPage {
  std::size_t page_index = 0;
  Size size{.width = 256.0F, .height = 256.0F};
  Point cursor;
  float row_height = 0.0F;
  std::vector<GlyphAtlasEntry> entries;
};

struct TexturedGlyphQuad {
  GlyphAtlasKey key;
  std::size_t page_index = 0;
  Rect device_bounds;
  Rect atlas_bounds;
  Rect atlas_uv_bounds;
  Color color;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

} // namespace cgpui
