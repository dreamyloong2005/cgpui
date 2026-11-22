#include "text_shaping_internal.hpp"

#include <string>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

char32_t decode_utf8_codepoint(
    std::string_view text,
    std::size_t byte_offset,
    std::size_t byte_length) {
  if (byte_offset >= text.size() || byte_length == 0) {
    return U'\0';
  }

  const auto byte_at = [text](std::size_t index) {
    return static_cast<unsigned char>(text[index]);
  };
  const unsigned char first = byte_at(byte_offset);
  if (byte_length == 1 || (first & 0x80U) == 0U) {
    return static_cast<char32_t>(first);
  }
  if (byte_length == 2 && byte_offset + 1 < text.size()) {
    return static_cast<char32_t>(((first & 0x1FU) << 6U) |
                                 (byte_at(byte_offset + 1) & 0x3FU));
  }
  if (byte_length == 3 && byte_offset + 2 < text.size()) {
    return static_cast<char32_t>(((first & 0x0FU) << 12U) |
                                 ((byte_at(byte_offset + 1) & 0x3FU) << 6U) |
                                 (byte_at(byte_offset + 2) & 0x3FU));
  }
  if (byte_length == 4 && byte_offset + 3 < text.size()) {
    return static_cast<char32_t>(((first & 0x07U) << 18U) |
                                 ((byte_at(byte_offset + 1) & 0x3FU) << 12U) |
                                 ((byte_at(byte_offset + 2) & 0x3FU) << 6U) |
                                 (byte_at(byte_offset + 3) & 0x3FU));
  }
  return static_cast<char32_t>(first);
}

std::size_t select_font_fallback_face_index(
    const std::vector<FontFaceDescriptor>& faces,
    char32_t codepoint) {
  for (std::size_t index = 0; index < faces.size(); ++index) {
    if (font_face_covers_codepoint(faces[index], codepoint)) {
      return index;
    }
  }
  return 0;
}

bool font_fallback_faces_have_known_miss(
    const std::vector<FontFaceDescriptor>& faces,
    char32_t codepoint) {
  if (faces.empty()) {
    return false;
  }
  for (const FontFaceDescriptor& face : faces) {
    if (!font_face_declares_coverage(face)) {
      return false;
    }
    if (font_face_covers_codepoint(face, codepoint)) {
      return false;
    }
  }
  return true;
}

bool codepoint_prefers_color_glyph(char32_t codepoint) {
  return 0x1F000 <= codepoint && codepoint <= 0x1FAFF;
}

TextShapingScript classify_text_shaping_script(char32_t codepoint) {
  if (codepoint_prefers_color_glyph(codepoint)) {
    return TextShapingScript::emoji;
  }
  if (('A' <= codepoint && codepoint <= 'Z') ||
      ('a' <= codepoint && codepoint <= 'z')) {
    return TextShapingScript::latin;
  }
  if (0x4E00 <= codepoint && codepoint <= 0x9FFF) {
    return TextShapingScript::han;
  }
  if (0x3040 <= codepoint && codepoint <= 0x309F) {
    return TextShapingScript::hiragana;
  }
  if (0x30A0 <= codepoint && codepoint <= 0x30FF) {
    return TextShapingScript::katakana;
  }
  if (0xAC00 <= codepoint && codepoint <= 0xD7AF) {
    return TextShapingScript::hangul;
  }
  if (0x0590 <= codepoint && codepoint <= 0x05FF) {
    return TextShapingScript::hebrew;
  }
  if (0x0600 <= codepoint && codepoint <= 0x06FF) {
    return TextShapingScript::arabic;
  }
  if (0x0900 <= codepoint && codepoint <= 0x097F) {
    return TextShapingScript::devanagari;
  }
  return TextShapingScript::common;
}

bool is_emoji_presentation_selector(char32_t codepoint) {
  return codepoint == 0xFE0F;
}

bool codepoint_accepts_emoji_presentation(char32_t codepoint) {
  return codepoint_prefers_color_glyph(codepoint) ||
         codepoint == 0x00A9 || codepoint == 0x00AE ||
         codepoint == 0x203C || codepoint == 0x2049 ||
         codepoint == 0x2122 || codepoint == 0x2139 ||
         (0x2194 <= codepoint && codepoint <= 0x21AA) ||
         (0x231A <= codepoint && codepoint <= 0x231B) ||
         codepoint == 0x2328 || codepoint == 0x23CF ||
         (0x23E9 <= codepoint && codepoint <= 0x23F3) ||
         (0x23F8 <= codepoint && codepoint <= 0x23FA) ||
         codepoint == 0x24C2 ||
         (0x25AA <= codepoint && codepoint <= 0x25AB) ||
         codepoint == 0x25B6 || codepoint == 0x25C0 ||
         (0x25FB <= codepoint && codepoint <= 0x25FE) ||
         (0x2600 <= codepoint && codepoint <= 0x27BF) ||
         (0x2934 <= codepoint && codepoint <= 0x2935) ||
         (0x2B05 <= codepoint && codepoint <= 0x2B55) ||
         codepoint == 0x3030 || codepoint == 0x303D ||
         codepoint == 0x3297 || codepoint == 0x3299;
}

bool is_emoji_sequence_joiner(char32_t codepoint) {
  return codepoint == 0x200D;
}

char32_t next_utf8_codepoint_after(
    std::string_view text,
    std::size_t byte_offset) {
  if (byte_offset >= text.size()) {
    return U'\0';
  }
  std::size_t byte_length = 1;
  while (byte_offset + byte_length < text.size() &&
         is_utf8_continuation_byte(text[byte_offset + byte_length])) {
    byte_length += 1;
  }
  return decode_utf8_codepoint(text, byte_offset, byte_length);
}

bool is_emoji_sequence_joiner_between_emoji(
    char32_t codepoint,
    bool has_previous_codepoint,
    char32_t previous_codepoint,
    char32_t next_codepoint) {
  return is_emoji_sequence_joiner(codepoint) && has_previous_codepoint &&
         codepoint_accepts_emoji_presentation(previous_codepoint) &&
         codepoint_accepts_emoji_presentation(next_codepoint);
}

void append_font_fallback_run_span(
    TextShapeRun& run,
    std::size_t font_fallback_face_index,
    std::size_t glyph_index,
    std::size_t byte_offset,
    std::size_t byte_length,
    float advance,
    float device_advance) {
  if (run.font_runs.empty() ||
      run.font_runs.back().font_fallback_face_index !=
          font_fallback_face_index) {
    run.font_runs.push_back(TextFontFallbackRun{
        .font_fallback_face_index = font_fallback_face_index,
        .glyph_start = glyph_index,
        .glyph_end = glyph_index,
        .byte_start = byte_offset,
        .byte_end = byte_offset,
    });
  }

  TextFontFallbackRun& font_run = run.font_runs.back();
  font_run.glyph_end = glyph_index + 1;
  font_run.byte_end = byte_offset + byte_length;
  font_run.advance += advance;
  font_run.device_advance += device_advance;
}

void append_script_run_span(
    TextShapeRun& run,
    TextShapingScript script,
    std::size_t glyph_index,
    std::size_t byte_offset,
    std::size_t byte_length,
    float advance,
    float device_advance) {
  if (run.script_runs.empty() || run.script_runs.back().script != script) {
    run.script_runs.push_back(TextScriptRun{
        .script = script,
        .glyph_start = glyph_index,
        .glyph_end = glyph_index,
        .byte_start = byte_offset,
        .byte_end = byte_offset,
    });
  }

  TextScriptRun& script_run = run.script_runs.back();
  script_run.glyph_end = glyph_index + 1;
  script_run.byte_end = byte_offset + byte_length;
  script_run.advance += advance;
  script_run.device_advance += device_advance;
}

void append_missing_glyph_diagnostic(
    TextShapeRun& run,
    char32_t codepoint,
    std::size_t glyph_index,
    std::size_t byte_offset,
    std::size_t byte_length,
    std::size_t font_fallback_face_index) {
  run.missing_glyphs.push_back(TextMissingGlyphDiagnostic{
      .codepoint = codepoint,
      .glyph_index = glyph_index,
      .byte_offset = byte_offset,
      .byte_length = byte_length,
      .font_fallback_face_index = font_fallback_face_index,
  });
}

void append_color_glyph_plan(
    TextShapeRun& run,
    char32_t codepoint,
    std::size_t glyph_index,
    std::size_t byte_offset,
    std::size_t byte_length,
    std::size_t font_fallback_face_index) {
  run.color_glyphs.push_back(TextColorGlyphPlan{
      .codepoint = codepoint,
      .glyph_index = glyph_index,
      .byte_offset = byte_offset,
      .byte_length = byte_length,
      .font_fallback_face_index = font_fallback_face_index,
      .format = TextColorGlyphFormat::native_color,
  });
}

void mark_emoji_presentation_selector_span(
    TextColorGlyphPlan& plan,
    std::size_t selector_byte_offset,
    std::size_t selector_byte_length) {
  plan.has_emoji_presentation_selector = true;
  plan.emoji_presentation_selector_byte_offset = selector_byte_offset;
  plan.emoji_presentation_selector_byte_length = selector_byte_length;
}

void append_emoji_presentation_color_glyph_plan(
    TextShapeRun& run,
    char32_t previous_codepoint,
    std::size_t previous_glyph_index,
    std::size_t previous_byte_offset,
    std::size_t previous_byte_length,
    std::size_t previous_font_fallback_face_index,
    std::size_t selector_byte_offset,
    std::size_t selector_byte_length) {
  if (!codepoint_accepts_emoji_presentation(previous_codepoint)) {
    return;
  }
  if (!run.color_glyphs.empty() &&
      run.color_glyphs.back().glyph_index == previous_glyph_index) {
    mark_emoji_presentation_selector_span(
        run.color_glyphs.back(),
        selector_byte_offset,
        selector_byte_length);
    return;
  }
  append_color_glyph_plan(
      run,
      previous_codepoint,
      previous_glyph_index,
      previous_byte_offset,
      previous_byte_length,
      previous_font_fallback_face_index);
  mark_emoji_presentation_selector_span(
      run.color_glyphs.back(),
      selector_byte_offset,
      selector_byte_length);
}

} // namespace

TextShapeRun shape_text_with_deterministic_fallback(
    TextShapingRequest request) {
  const float scale_value = normalized_scale(request.scale);
  TextShapeRun run{
      .text = std::string(request.text),
      .font = std::move(request.font),
      .font_fallback_faces = std::move(request.font_fallback_faces),
      .font_size = request.font_size,
      .scale = request.scale,
      .byte_length = request.text.size(),
      .line_height = request.font_size,
      .device_font_size = request.font_size * scale_value,
      .device_line_height = request.font_size * scale_value,
      .requested_backend = request.backend.requested,
      .used_backend = request.backend.used,
      .fallback_reason = request.backend.fallback_reason,
      .backend_capabilities = request.backend.capabilities,
      .direction = request.direction,
      .script = request.script,
      .language = std::move(request.language),
  };

  const float fallback_advance = request.font_size * 0.5F;
  run.glyphs.reserve(request.text.size());
  run.font_runs.reserve(request.text.size());
  run.script_runs.reserve(request.text.size());
  run.missing_glyphs.reserve(request.text.size());
  run.color_glyphs.reserve(request.text.size());
  std::size_t byte_offset = 0;
  std::uint32_t glyph_id = 0;
  bool has_previous_codepoint = false;
  char32_t previous_codepoint = U'\0';
  std::size_t previous_glyph_index = 0;
  std::size_t previous_byte_offset = 0;
  std::size_t previous_byte_length = 0;
  std::size_t previous_font_fallback_face_index = 0;
  while (byte_offset < request.text.size()) {
    std::size_t byte_length = 1;
    while (byte_offset + byte_length < request.text.size() &&
           is_utf8_continuation_byte(request.text[byte_offset + byte_length])) {
      byte_length += 1;
    }
    const char32_t codepoint =
        decode_utf8_codepoint(request.text, byte_offset, byte_length);
    const bool emoji_presentation_selector =
        is_emoji_presentation_selector(codepoint);
    const char32_t next_codepoint =
        next_utf8_codepoint_after(request.text, byte_offset + byte_length);
    const bool emoji_sequence_joiner =
        is_emoji_sequence_joiner_between_emoji(
            codepoint,
            has_previous_codepoint,
            previous_codepoint,
            next_codepoint);
    const std::size_t font_fallback_face_index =
        select_font_fallback_face_index(run.font_fallback_faces, codepoint);
    const std::size_t glyph_index = run.glyphs.size();
    const float device_advance = fallback_advance * scale_value;
    run.glyphs.push_back(TextGlyphRun{
        .glyph_id = glyph_id,
        .byte_offset = byte_offset,
        .byte_length = byte_length,
        .font_fallback_face_index = font_fallback_face_index,
        .advance = fallback_advance,
    });
    run.total_advance += fallback_advance;
    run.device_total_advance += device_advance;
    append_font_fallback_run_span(
        run,
        font_fallback_face_index,
        glyph_index,
        byte_offset,
        byte_length,
        fallback_advance,
        device_advance);
    append_script_run_span(
        run,
        classify_text_shaping_script(codepoint),
        glyph_index,
        byte_offset,
        byte_length,
        fallback_advance,
        device_advance);
    if (!emoji_presentation_selector && !emoji_sequence_joiner &&
        font_fallback_faces_have_known_miss(
            run.font_fallback_faces,
            codepoint)) {
      append_missing_glyph_diagnostic(
          run,
          codepoint,
          glyph_index,
          byte_offset,
          byte_length,
          font_fallback_face_index);
    }
    if (emoji_presentation_selector && has_previous_codepoint) {
      append_emoji_presentation_color_glyph_plan(
          run,
          previous_codepoint,
          previous_glyph_index,
          previous_byte_offset,
          previous_byte_length,
          previous_font_fallback_face_index,
          byte_offset,
          byte_length);
    }
    if (codepoint_prefers_color_glyph(codepoint)) {
      append_color_glyph_plan(
          run,
          codepoint,
          glyph_index,
          byte_offset,
          byte_length,
          font_fallback_face_index);
    }
    if (!emoji_presentation_selector && !is_emoji_sequence_joiner(codepoint)) {
      has_previous_codepoint = true;
      previous_codepoint = codepoint;
      previous_glyph_index = glyph_index;
      previous_byte_offset = byte_offset;
      previous_byte_length = byte_length;
      previous_font_fallback_face_index = font_fallback_face_index;
    }
    byte_offset += byte_length;
    glyph_id += 1;
  }
  return run;
}

} // namespace cgpui
