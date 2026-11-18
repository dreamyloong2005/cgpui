#include "text_glyph_id_raster_internal.hpp"

#include "text_dwrite_win32_internal.hpp"

#include <utility>
#include <vector>

namespace cgpui {
namespace {

Error raster_error(const char* message) {
  return Error{
      .code = ErrorCode::renderer_initialization_failed,
      .message = std::string("DirectWrite glyph-id rasterization: ") + message,
  };
}

RasterizedGlyph empty_glyph(const TextGlyphPaint& glyph) {
  return RasterizedGlyph{
      .key = glyph.key,
      .bitmap = {.width = 1, .height = 1, .stride = 1, .alpha = {0}},
      .advance = glyph.device_advance,
      .device_font_size = glyph.key.device_font_size,
  };
}

} // namespace

Result<RasterizedGlyph> rasterize_native_glyph_id(const TextGlyphPaint& glyph) {
  const GlyphAtlasKey& key = glyph.key;
  if (key.glyph_id > 0xFFFFU) {
    return std::unexpected(raster_error("glyph id exceeds 16 bits"));
  }
  const ComPtr<IDWriteFontFace> face = dwrite_face_from_identity(
      key.font_face_path, key.font_face_index, key.synthetic_bold,
      key.synthetic_oblique);
  IDWriteFactory2* factory = dwrite_factory();
  if (!face || factory == nullptr) {
    return std::unexpected(raster_error("font face is unavailable"));
  }
  const auto index = static_cast<UINT16>(key.glyph_id);
  const FLOAT advance = 0.0F;
  const DWRITE_GLYPH_OFFSET offset{};
  const DWRITE_GLYPH_RUN run{
      .fontFace = face.Get(),
      .fontEmSize = key.font_size,
      .glyphCount = 1,
      .glyphIndices = &index,
      .glyphAdvances = &advance,
      .glyphOffsets = &offset,
      .isSideways = FALSE,
      .bidiLevel = 0,
  };
  ComPtr<IDWriteGlyphRunAnalysis> analysis;
  // The pen sits at pixel 0; the subpixel bucket shifts the outline right.
  const float origin_x = static_cast<float>(key.subpixel_x) * 0.25F;
  if (FAILED(factory->CreateGlyphRunAnalysis(&run, key.scale, nullptr,
          DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC, DWRITE_MEASURING_MODE_NATURAL,
          origin_x, 0.0F, analysis.GetAddressOf()))) {
    return std::unexpected(raster_error("glyph run analysis failed"));
  }
  RECT bounds{};
  if (FAILED(analysis->GetAlphaTextureBounds(DWRITE_TEXTURE_CLEARTYPE_3x1, &bounds))) {
    return std::unexpected(raster_error("texture bounds failed"));
  }
  if (bounds.right <= bounds.left || bounds.bottom <= bounds.top) {
    return empty_glyph(glyph);
  }
  const auto width = static_cast<std::uint32_t>(bounds.right - bounds.left);
  const auto height = static_cast<std::uint32_t>(bounds.bottom - bounds.top);
  if (width > 4096U || height > 4096U) {
    return std::unexpected(raster_error("glyph bitmap exceeds 4096 pixels"));
  }
  std::vector<std::uint8_t> samples(static_cast<std::size_t>(width) * height * 3U);
  if (FAILED(analysis->CreateAlphaTexture(DWRITE_TEXTURE_CLEARTYPE_3x1, &bounds,
          samples.data(), static_cast<UINT32>(samples.size())))) {
    return std::unexpected(raster_error("alpha texture failed"));
  }
  GlyphBitmap bitmap{.width = width, .height = height, .stride = width};
  bitmap.alpha.resize(static_cast<std::size_t>(width) * height);
  for (std::size_t pixel = 0; pixel < bitmap.alpha.size(); ++pixel) {
    // Collapse ClearType channels to grayscale coverage for the alpha atlas.
    const unsigned sum = static_cast<unsigned>(samples[pixel * 3U]) +
        samples[pixel * 3U + 1U] + samples[pixel * 3U + 2U];
    bitmap.alpha[pixel] = static_cast<std::uint8_t>((sum + 1U) / 3U);
  }
  return RasterizedGlyph{
      .key = key,
      .bitmap = std::move(bitmap),
      .advance = glyph.device_advance,
      .device_font_size = key.device_font_size,
      .left_bearing = static_cast<float>(bounds.left),
      .top_bearing = static_cast<float>(bounds.top),
      .baseline = 0.0F,
  };
}

} // namespace cgpui
