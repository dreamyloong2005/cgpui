#include "text_shaping_native_internal.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace cgpui {
namespace {

bool same_face(const FontFaceDescriptor& a, const FontFaceDescriptor& b) {
  return a.path == b.path && a.face_index == b.face_index &&
      a.synthetic_bold == b.synthetic_bold &&
      a.synthetic_oblique == b.synthetic_oblique;
}

bool is_control(char32_t codepoint) {
  return codepoint < 0x20U || codepoint == 0x7FU;
}

bool never_missing(char32_t codepoint) {
  return is_control(codepoint) || codepoint == U' ' || codepoint == 0x200DU ||
      (0xFE00U <= codepoint && codepoint <= 0xFE0FU);
}

bool prefers_color_glyph(char32_t codepoint) {
  return 0x1F000U <= codepoint && codepoint <= 0x1FAFFU;
}

bool has_feature(const FontDescriptor& font, const char (&tag)[5]) {
  return std::ranges::any_of(font.features, [&](const FontFeature& feature) {
    return feature.tag == font_feature(tag, 0).tag;
  });
}

void append_font_span(
    TextShapeRun& run,
    std::size_t slot,
    const TextFontMetrics& metrics,
    std::size_t glyph_index,
    const TextGlyphRun& glyph,
    float scale) {
  if (run.font_runs.empty() ||
      run.font_runs.back().font_fallback_face_index != slot) {
    run.font_runs.push_back(TextFontFallbackRun{
        .font_fallback_face_index = slot,
        .metrics = metrics,
        .glyph_start = glyph_index,
        .glyph_end = glyph_index,
        .byte_start = glyph.byte_offset,
        .byte_end = glyph.byte_offset,
    });
  }
  TextFontFallbackRun& span = run.font_runs.back();
  span.glyph_end = glyph_index + 1U;
  span.byte_end = std::max(span.byte_end, glyph.byte_offset + glyph.byte_length);
  span.advance += glyph.advance;
  span.device_advance += glyph.advance * scale;
}

void append_script_span(
    TextShapeRun& run,
    TextShapingScript script,
    std::size_t glyph_index,
    const TextGlyphRun& glyph,
    float scale) {
  if (run.script_runs.empty() || run.script_runs.back().script != script) {
    run.script_runs.push_back(TextScriptRun{
        .script = script,
        .glyph_start = glyph_index,
        .glyph_end = glyph_index,
        .byte_start = glyph.byte_offset,
        .byte_end = glyph.byte_offset,
    });
  }
  TextScriptRun& span = run.script_runs.back();
  span.glyph_end = glyph_index + 1U;
  span.byte_end = std::max(span.byte_end, glyph.byte_offset + glyph.byte_length);
  span.advance += glyph.advance;
  span.device_advance += glyph.advance * scale;
}

} // namespace

NativeShapeBuilder::NativeShapeBuilder(
    TextShapingRequest& request,
    TextShapingBackend used)
    : request_(request), used_(used) {
  glyphs_.reserve(request.text.size());
}

std::size_t NativeShapeBuilder::add_face(NativeShapedFace face) {
  for (std::size_t slot = 0; slot < faces_.size(); ++slot) {
    if (same_face(faces_[slot].face, face.face)) {
      return slot;
    }
  }
  faces_.push_back(std::move(face));
  return faces_.size() - 1U;
}

void NativeShapeBuilder::add_glyph(
    std::size_t face_slot,
    NativeShapedGlyph glyph) {
  glyphs_.emplace_back(face_slot, glyph);
}

TextShapeRun NativeShapeBuilder::finish() {
  const float scale = normalized_scale(request_.scale);
  TextShapeRun run{
      .text = std::string(request_.text),
      .font = std::move(request_.font),
      .font_size = request_.font_size,
      .scale = request_.scale,
      .byte_length = request_.text.size(),
      .device_font_size = request_.font_size * scale,
      .requested_backend = request_.backend.requested,
      .used_backend = used_,
      .fallback_reason = TextShapingFallbackReason::none,
      .backend_capabilities = request_.backend.capabilities,
      .direction = request_.direction,
      .script = request_.script,
      .language = std::move(request_.language),
  };
  run.font_fallback_faces.reserve(faces_.size());
  for (const NativeShapedFace& face : faces_) {
    run.font_fallback_faces.push_back(face.face);
  }
  if (!faces_.empty()) {
    run.font_metrics = faces_.front().metrics;
  }
  const float space = !faces_.empty() && faces_.front().space_advance > 0.0F
      ? faces_.front().space_advance
      : request_.font_size * 0.5F;
  const float tab_stop = std::max(1.0F, space * request_.tab_width_in_spaces);

  run.glyphs.reserve(glyphs_.size());
  float x = 0.0F;
  std::size_t previous_cluster = run.text.size() + 1U;
  for (auto& [slot, shaped] : glyphs_) {
    std::size_t length = 0;
    const char32_t codepoint =
        text_shaping_decode_utf8(run.text, shaped.cluster_start, &length);
    const NativeShapedFace& face = faces_[slot];
    if (codepoint == U'\t') {
      shaped.glyph_id = face.space_glyph_id;
      shaped.advance = (std::floor(x / tab_stop + 1.0e-4F) + 1.0F) * tab_stop - x;
      shaped.offset = {};
    } else if (is_control(codepoint)) {
      shaped.glyph_id = face.space_glyph_id;
      shaped.advance = 0.0F;
      shaped.offset = {};
    }
    const std::size_t index = run.glyphs.size();
    run.glyphs.push_back(TextGlyphRun{
        .glyph_id = shaped.glyph_id,
        .byte_offset = shaped.cluster_start,
        .byte_length = shaped.cluster_end - shaped.cluster_start,
        .font_fallback_face_index = slot,
        .advance = shaped.advance,
        .offset = shaped.offset,
    });
    const TextGlyphRun& glyph = run.glyphs.back();
    x += glyph.advance;
    run.total_advance += glyph.advance;
    run.device_total_advance += glyph.advance * scale;
    append_font_span(run, slot, face.metrics, index, glyph, scale);
    append_script_span(
        run, text_shaping_script_for_codepoint(codepoint), index, glyph, scale);
    const bool cluster_start = shaped.cluster_start != previous_cluster;
    previous_cluster = shaped.cluster_start;
    if (glyph.glyph_id == 0 && !never_missing(codepoint)) {
      run.missing_glyphs.push_back(TextMissingGlyphDiagnostic{
          .codepoint = codepoint,
          .glyph_index = index,
          .byte_offset = glyph.byte_offset,
          .byte_length = glyph.byte_length,
          .font_fallback_face_index = slot,
      });
    }
    if (cluster_start && prefers_color_glyph(codepoint)) {
      run.color_glyphs.push_back(TextColorGlyphPlan{
          .codepoint = codepoint,
          .glyph_index = index,
          .byte_offset = glyph.byte_offset,
          .byte_length = glyph.byte_length,
          .font_fallback_face_index = slot,
      });
    }
  }
  const TextLineMetrics line = native_text_line_metrics(run);
  run.line_height = line.line_height;
  run.device_line_height = line.device_line_height;
  return run;
}

std::vector<FontFeature> native_font_features(
    const FontDescriptor& font,
    bool monospaced) {
  std::vector<FontFeature> features = font.features;
  if (monospaced && !has_feature(font, "liga")) {
    features.push_back(font_feature("liga", 0));
  }
  if (monospaced && !has_feature(font, "calt")) {
    features.push_back(font_feature("calt", 0));
  }
  return features;
}

} // namespace cgpui
