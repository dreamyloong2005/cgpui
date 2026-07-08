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
  run.missing_glyphs.reserve(request.text.size());
  std::size_t byte_offset = 0;
  std::uint32_t glyph_id = 0;
  while (byte_offset < request.text.size()) {
    std::size_t byte_length = 1;
    while (byte_offset + byte_length < request.text.size() &&
           is_utf8_continuation_byte(request.text[byte_offset + byte_length])) {
      byte_length += 1;
    }
    const char32_t codepoint =
        decode_utf8_codepoint(request.text, byte_offset, byte_length);
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
    if (font_fallback_faces_have_known_miss(
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
    byte_offset += byte_length;
    glyph_id += 1;
  }
  return run;
}

} // namespace cgpui
