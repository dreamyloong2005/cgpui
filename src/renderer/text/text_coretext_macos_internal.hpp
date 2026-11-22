#pragma once

#include "cgpui/platform/font_database.hpp"
#include "cgpui/renderer/text/text_shape.hpp"

#include <CoreText/CoreText.h>

#include <cstdint>
#include <string_view>

namespace cgpui {

// Core Text faces are identified in-process: the font file path plus a
// registry slot in the upper 16 bits of face_index ((slot + 1) << 16).
// Variable system fonts (San Francisco) have several instances in one
// file, so the path alone cannot recreate the weight that was shaped.
[[nodiscard]] std::uint32_t core_text_register_face(CTFontRef font);

// A new font for a registered face at `size` (caller releases), or
// nullptr for an unknown face index.
[[nodiscard]] CTFontRef core_text_font_for_face(
    std::uint32_t face_index,
    float size);

// The primary font of a request: family (or the system UI font for
// "system-ui"), CSS weight, style and OpenType features.
[[nodiscard]] CTFontRef core_text_primary_font(
    const FontDescriptor& font,
    float size);

// Identity, monospace flag and metrics of a font, for NativeShapeBuilder.
[[nodiscard]] FontFaceDescriptor core_text_face_descriptor(
    CTFontRef font,
    const FontDescriptor& requested);
[[nodiscard]] TextFontMetrics core_text_font_metrics(CTFontRef font);

} // namespace cgpui
