#include "text_glyph_id_raster_internal.hpp"

#include "text_fontconfig_linux_internal.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYNTHESIS_H

#include <cstring>
#include <utility>

namespace cgpui {
namespace {

Error raster_error(const char* message) {
  return Error{
      .code = ErrorCode::renderer_initialization_failed,
      .message = std::string("FreeType glyph-id rasterization: ") + message,
  };
}

} // namespace

Result<RasterizedGlyph> rasterize_native_glyph_id(const TextGlyphPaint& glyph) {
  const GlyphAtlasKey& key = glyph.key;
  const std::lock_guard lock(linux_ft_mutex());
  auto* face = static_cast<FT_Face>(
      linux_ft_face_locked(key.font_face_path, key.font_face_index));
  if (face == nullptr) {
    return std::unexpected(raster_error("font face is unavailable"));
  }
  const auto size = static_cast<FT_F26Dot6>(key.device_font_size * 64.0F);
  if (FT_Set_Char_Size(face, 0, size, 72, 72) != 0) {
    return std::unexpected(raster_error("font size rejected"));
  }
  // Quarter-pixel subpixel shift, plus a 12-degree shear for synthetic
  // oblique faces.
  FT_Vector delta{static_cast<FT_Pos>(key.subpixel_x) * 16, 0};
  FT_Matrix shear{0x10000, 0x0366A, 0, 0x10000};
  FT_Set_Transform(face, key.synthetic_oblique ? &shear : nullptr, &delta);
  const FT_Error loaded =
      FT_Load_Glyph(face, key.glyph_id, FT_LOAD_TARGET_LIGHT | FT_LOAD_NO_BITMAP);
  if (loaded == 0 && key.synthetic_bold) {
    FT_GlyphSlot_Embolden(face->glyph);
  }
  const FT_Error rendered =
      loaded == 0 ? FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL) : loaded;
  FT_Set_Transform(face, nullptr, nullptr);
  if (rendered != 0) {
    return std::unexpected(raster_error("glyph could not be rendered"));
  }
  const FT_Bitmap& source = face->glyph->bitmap;
  RasterizedGlyph output{
      .key = key,
      .bitmap = {.width = 1, .height = 1, .stride = 1, .alpha = {0}},
      .advance = glyph.device_advance,
      .device_font_size = key.device_font_size,
  };
  if (source.width == 0 || source.rows == 0 || source.buffer == nullptr) {
    return output;
  }
  if (source.width > 4096U || source.rows > 4096U ||
      source.pixel_mode != FT_PIXEL_MODE_GRAY) {
    return std::unexpected(raster_error("unexpected bitmap format or size"));
  }
  GlyphBitmap bitmap{.width = source.width, .height = source.rows, .stride = source.width};
  bitmap.alpha.resize(static_cast<std::size_t>(source.width) * source.rows);
  for (unsigned int row = 0; row < source.rows; ++row) {
    const unsigned char* line = source.pitch >= 0
        ? source.buffer + static_cast<std::ptrdiff_t>(row) * source.pitch
        : source.buffer + static_cast<std::ptrdiff_t>(source.rows - 1U - row) * -source.pitch;
    std::memcpy(bitmap.alpha.data() + static_cast<std::size_t>(row) * source.width, line,
                source.width);
  }
  output.bitmap = std::move(bitmap);
  output.left_bearing = static_cast<float>(face->glyph->bitmap_left);
  output.top_bearing = -static_cast<float>(face->glyph->bitmap_top);
  return output;
}

} // namespace cgpui
