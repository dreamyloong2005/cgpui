#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/ui/text_glyphs.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <dwrite.h>

namespace cgpui {

[[nodiscard]] Result<GlyphBitmap> win32_text_glyph_coverage(
    IDWriteFactory* factory,
    IDWriteTextLayout* layout,
    float scale,
    float horizontal_fit,
    float vertical_fit,
    std::uint32_t width,
    std::uint32_t height);

} // namespace cgpui
