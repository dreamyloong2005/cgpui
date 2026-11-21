#include "cgpui/ui/text_glyph_rasterizer.hpp"

#if defined(_WIN32)
#include "text_glyph_raster_win32_internal.hpp"
#endif
#include "text_glyph_id_raster_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {

Result<RasterizedGlyph> rasterize_text_glyph(
    const TextGlyphPaint& glyph,
    std::string_view utf8) {
  if (!std::isfinite(glyph.device_advance) || glyph.device_advance < 0.0F ||
      glyph.device_advance > 4096.0F ||
      !std::isfinite(glyph.key.device_font_size) ||
      glyph.key.device_font_size <= 0.0F ||
      glyph.key.device_font_size > 4096.0F ||
      !std::isfinite(glyph.key.font_size) || glyph.key.font_size <= 0.0F ||
      glyph.key.font_size > 4096.0F ||
      !std::isfinite(glyph.key.scale) || glyph.key.scale <= 0.0F ||
      glyph.key.scale > 64.0F ||
      utf8.size() > 16384) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "glyph rasterization requires finite, bounded dimensions and UTF-8 input",
    });
  }
  const TextShapingBackend backend = glyph.key.shaping_backend;
#if defined(_WIN32)
  if (backend == TextShapingBackend::directwrite) {
    return rasterize_native_glyph_id(glyph);
  }
#elif defined(__APPLE__)
  if (backend == TextShapingBackend::core_text) {
    return rasterize_native_glyph_id(glyph);
  }
#elif defined(CGPUI_HAS_FREETYPE_GLYPH_RASTERIZER)
  if (backend == TextShapingBackend::harfbuzz) {
    return rasterize_native_glyph_id(glyph);
  }
#endif
  if (utf8.empty()) {
    const auto width = rasterized_glyph_dimension(glyph.device_advance);
    const auto height = rasterized_glyph_dimension(glyph.key.device_font_size);
    return RasterizedGlyph{
        .key = glyph.key,
        .bitmap = {.width = width, .height = height, .stride = width,
                   .alpha = std::vector<std::uint8_t>(static_cast<std::size_t>(width) * height)},
        .advance = glyph.device_advance,
        .device_font_size = glyph.key.device_font_size,
        .baseline = glyph.key.device_font_size * 0.8F,
    };
  }
#if defined(_WIN32)
  if (backend != TextShapingBackend::deterministic_fallback) {
    return std::unexpected(Error{
        .code = ErrorCode::unsupported_platform,
        .message = "DirectWrite rasterizes DirectWrite glyph ids or deterministic clusters, not HarfBuzz glyph runs",
    });
  }
  return rasterize_win32_text_glyph(glyph, utf8);
#else
  RasterizedGlyph result = rasterize_fallback_glyph(glyph);
  if (std::ranges::all_of(utf8, [](char value) {
        return value == ' ' || value == '\t' || value == '\n' || value == '\r';
      })) {
    std::ranges::fill(result.bitmap.alpha, 0);
  }
  return result;
#endif
}

} // namespace cgpui
