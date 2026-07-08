#pragma once

#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/text_font.hpp"
#include "cgpui/ui/text_shaping_backend.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

struct TextGlyphRun {
  std::uint32_t glyph_id = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_length = 0;
  std::size_t font_fallback_face_index = 0;
  float advance = 0.0F;
  Point offset;
};

struct TextMissingGlyphDiagnostic {
  char32_t codepoint = U'\0';
  std::size_t glyph_index = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_length = 0;
  std::size_t font_fallback_face_index = 0;
};

enum class TextColorGlyphFormat {
  native_color,
};

struct TextColorGlyphPlan {
  char32_t codepoint = U'\0';
  std::size_t glyph_index = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_length = 0;
  std::size_t font_fallback_face_index = 0;
  TextColorGlyphFormat format = TextColorGlyphFormat::native_color;
  bool has_emoji_presentation_selector = false;
  std::size_t emoji_presentation_selector_byte_offset = 0;
  std::size_t emoji_presentation_selector_byte_length = 0;
};

struct TextFontFallbackRun {
  std::size_t font_fallback_face_index = 0;
  std::size_t glyph_start = 0;
  std::size_t glyph_end = 0;
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
  float advance = 0.0F;
  float device_advance = 0.0F;
};

struct TextShapeRun {
  std::string text;
  FontDescriptor font;
  std::vector<FontFaceDescriptor> font_fallback_faces;
  std::vector<TextFontFallbackRun> font_runs;
  std::vector<TextMissingGlyphDiagnostic> missing_glyphs;
  std::vector<TextColorGlyphPlan> color_glyphs;
  float font_size = 16.0F;
  DpiScale scale;
  std::vector<TextGlyphRun> glyphs;
  std::size_t byte_length = 0;
  float total_advance = 0.0F;
  float line_height = 16.0F;
  float device_font_size = 16.0F;
  float device_total_advance = 0.0F;
  float device_line_height = 16.0F;
  TextShapingBackend requested_backend = TextShapingBackend::harfbuzz;
  TextShapingBackend used_backend = TextShapingBackend::deterministic_fallback;
  TextShapingFallbackReason fallback_reason =
      TextShapingFallbackReason::backend_unavailable;
  TextShapingBackendCapabilities backend_capabilities;
  TextShapingDirection direction = TextShapingDirection::left_to_right;
  TextShapingScript script = TextShapingScript::common;
  std::string language;

  [[nodiscard]] std::size_t glyph_count() const;
  [[nodiscard]] bool used_fallback() const;
  [[nodiscard]] TextShapingBackendSelection backend_selection() const;
};

[[nodiscard]] bool is_utf8_continuation_byte(char value);

[[nodiscard]] TextShapeRun shape_text(
    std::string_view text,
    FontDescriptor font = {},
    float font_size = 16.0F,
    DpiScale scale = {},
    TextShapingOptions options = {});

[[nodiscard]] TextShapeRun shape_text(
    std::string_view text,
    const FontFallbackChain& fallback_chain,
    float font_size = 16.0F,
    DpiScale scale = {},
    TextShapingOptions options = {});

} // namespace cgpui
