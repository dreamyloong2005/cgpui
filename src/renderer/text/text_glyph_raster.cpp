#include "cgpui/renderer/text/text_glyphs.hpp"

#include "cgpui/renderer/text/text_measurement.hpp"
#include "text_glyph_paint_internal.hpp"

#include <algorithm>
#include <utility>

namespace cgpui {

bool GlyphBitmap::empty() const {
  return width == 0 || height == 0 || stride == 0 || alpha.empty();
}

std::size_t GlyphBitmap::byte_size() const {
  return alpha.size();
}

std::uint8_t GlyphBitmap::pixel(std::uint32_t x, std::uint32_t y) const {
  if (x >= width || y >= height || stride == 0) {
    return 0;
  }
  const std::size_t index =
      static_cast<std::size_t>(y) * static_cast<std::size_t>(stride) + x;
  return index < alpha.size() ? alpha[index] : 0;
}

std::uint32_t rasterized_glyph_dimension(float value) {
  if (value <= 0.0F) {
    return 1;
  }
  return std::max(1U, static_cast<std::uint32_t>(value + 0.5F));
}

RasterizedGlyph rasterize_fallback_glyph(
    const TextGlyphPaint& glyph,
    GlyphRasterizerOptions options) {
  const std::uint32_t width = rasterized_glyph_dimension(glyph.device_advance);
  const std::uint32_t height =
      rasterized_glyph_dimension(glyph.key.device_font_size);
  const std::uint32_t stride = width;
  std::vector<std::uint8_t> alpha(
      static_cast<std::size_t>(stride) * static_cast<std::size_t>(height),
      options.background_alpha);

  const bool can_pad =
      width > options.padding * 2U && height > options.padding * 2U;
  for (std::uint32_t y = 0; y < height; ++y) {
    for (std::uint32_t x = 0; x < width; ++x) {
      const bool inside = !can_pad ||
          (x >= options.padding && x + options.padding < width &&
           y >= options.padding && y + options.padding < height);
      if (inside) {
        alpha[static_cast<std::size_t>(y) * stride + x] =
            options.foreground_alpha;
      }
    }
  }

  return RasterizedGlyph{
      .key = glyph.key,
      .bitmap =
          GlyphBitmap{
              .width = width,
              .height = height,
              .stride = stride,
              .alpha = std::move(alpha),
          },
      .advance = glyph.device_advance,
      .device_font_size = glyph.key.device_font_size,
      .left_bearing = 0.0F,
      .top_bearing = 0.0F,
      .baseline = glyph.key.device_font_size * 0.8F,
  };
}

TextGlyphPaint text_glyph_paint_for(
    const TextShapeRun& run,
    std::size_t index,
    float pen_x,
    Point line_origin,
    float baseline) {
  const TextGlyphRun& glyph = run.glyphs[index];
  const float scale = normalized_scale(run.scale);
  const Point logical_origin{
      .x = pen_x + glyph.offset.x,
      .y = line_origin.y + glyph.offset.y,
  };
  TextGlyphPaint paint{
      .key =
          GlyphAtlasKey{
              .font_family = run.font.family,
              .font_size = run.font_size,
              .scale = scale,
              .device_font_size = run.device_font_size,
              .glyph_index = index,
              .glyph_id = glyph.glyph_id,
              .byte_offset = glyph.byte_offset,
              .byte_length = glyph.byte_length,
              .text = run.text.substr(glyph.byte_offset, glyph.byte_length),
              .shaping_backend = run.used_backend,
          },
      .origin = logical_origin,
      .advance = glyph.advance,
      .device_origin = to_device_pixels(logical_origin, run.scale),
      .device_advance = glyph.advance * scale,
  };
  const bool native =
      run.used_backend != TextShapingBackend::deterministic_fallback;
  if (native && glyph.font_fallback_face_index < run.font_fallback_faces.size()) {
    const FontFaceDescriptor& face =
        run.font_fallback_faces[glyph.font_fallback_face_index];
    paint.key.font_family = face.font.family;
    paint.key.font_face_path = face.path;
    paint.key.font_face_index = face.face_index;
    paint.key.synthetic_bold = face.synthetic_bold;
    paint.key.synthetic_oblique = face.synthetic_oblique;
    paint.device_baseline = baseline * scale;
  }
  return paint;
}

std::vector<TextGlyphPaint> text_glyph_paint_metadata(
    const TextShapeRun& run,
    Point origin) {
  std::vector<TextGlyphPaint> glyphs;
  glyphs.reserve(run.glyphs.size());
  const float baseline = text_line_metrics_for_shape_run(run).baseline;
  float x = origin.x;
  for (std::size_t index = 0; index < run.glyphs.size(); ++index) {
    glyphs.push_back(text_glyph_paint_for(run, index, x, origin, baseline));
    x += run.glyphs[index].advance;
  }
  return glyphs;
}

} // namespace cgpui
