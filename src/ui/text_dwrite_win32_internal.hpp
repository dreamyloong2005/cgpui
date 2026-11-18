#pragma once

#include "text_shaping_native_internal.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <dwrite_2.h>
#include <wrl/client.h>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

using Microsoft::WRL::ComPtr;

// Shared, free-threaded DirectWrite factory; null if creation failed.
[[nodiscard]] IDWriteFactory2* dwrite_factory();

struct DWriteUtf16Text {
  std::wstring text;
  // UTF-8 byte offset of every UTF-16 code unit, plus the end offset.
  std::vector<std::size_t> utf8_offsets;
};

[[nodiscard]] std::optional<DWriteUtf16Text> dwrite_utf16_from_utf8(
    std::string_view text);

[[nodiscard]] std::wstring dwrite_wide(std::string_view text);

[[nodiscard]] ComPtr<IDWriteFont> dwrite_find_font(
    std::string_view family,
    std::uint16_t weight,
    FontStyle style);

struct DWriteFace {
  ComPtr<IDWriteFontFace> face;
  NativeShapedFace info;
};

// Font face, file identity, metrics at `font_size` and the space glyph.
// Fails for faces that are not backed by a local font file.
[[nodiscard]] std::optional<DWriteFace> dwrite_face_for_font(
    IDWriteFont* font,
    float font_size);

// Face reopened from its file identity (cached), for glyph-id rasterization.
[[nodiscard]] ComPtr<IDWriteFontFace> dwrite_face_from_identity(
    const std::string& path,
    std::uint32_t face_index,
    bool synthetic_bold,
    bool synthetic_oblique);

struct DWriteItem {
  std::uint32_t start = 0;
  std::uint32_t length = 0;
  DWRITE_SCRIPT_ANALYSIS script{};
  std::uint8_t bidi_level = 0;
};

// Script (AnalyzeScript) and bidi (AnalyzeBidi) itemization.
[[nodiscard]] std::vector<DWriteItem> dwrite_itemize(
    const std::wstring& text,
    const std::wstring& locale,
    bool right_to_left);

struct DWriteFallbackSegment {
  std::uint32_t start = 0;
  std::uint32_t length = 0;
  ComPtr<IDWriteFont> font;  // null when no installed font covers it
  float scale = 1.0F;
};

// Font fallback (IDWriteFontFallback::MapCharacters) over one item. CJK
// ideographs prefer "Microsoft YaHei UI" unless the locale is ja or ko.
[[nodiscard]] std::vector<DWriteFallbackSegment> dwrite_map_fallback(
    const std::wstring& text,
    const std::wstring& locale,
    const DWriteItem& item,
    const std::wstring& family,
    IDWriteFont* base_font);

} // namespace cgpui
