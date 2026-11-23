#pragma once

#include "cgpui/renderer/glyph_atlas_types.hpp"
#include "cgpui/renderer/planning/text_positioning.hpp"
#include "cgpui/renderer/planning/text_vertex.hpp"

#include <span>
#include <vector>

namespace cgpui {

// Quad colors already include composed opacity; only positioning/transform
// is applied here. Glyph bounds and transform translations use device pixels.
[[nodiscard]] std::vector<TextVertex> build_text_vertices(
    std::span<const TexturedGlyphQuad> quads,
    TextPositioningPolicy positioning_policy = TextPositioningPolicy::preserve_subpixel);

} // namespace cgpui
