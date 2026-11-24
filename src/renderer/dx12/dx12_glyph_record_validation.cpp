#include "dx12_glyph_record_validation_internal.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace cgpui {
namespace {
bool coordinate(float value) {
  return std::isfinite(value) && value >= 0 && value <= 16384 && std::trunc(value) == value;
}
Error invalid_record() {
  return {.code = ErrorCode::invalid_argument, .message = "DX12 glyph record, page or payload is invalid"};
}
}
Result<Dx12GlyphRecordLayout> dx12_validate_glyph_record(
    const GlyphUploadRecord& upload, const GlyphAtlasPage& page,
    std::uint32_t width, std::uint32_t height) {
  const auto& key = upload.key;
  if (!width || !height || width > 16384 || height > 16384 ||
      page.page_index != upload.page_index || page.size.width != float(width) ||
      page.size.height != float(height) || !coordinate(upload.atlas_bounds.origin.x) ||
      !coordinate(upload.atlas_bounds.origin.y) || !upload.width || !upload.height ||
      upload.stride < upload.width || upload.atlas_bounds.size.width != float(upload.width) ||
      upload.atlas_bounds.size.height != float(upload.height) ||
      !std::isfinite(key.font_size) || key.font_size <= 0 || key.font_size > 4096 ||
      !std::isfinite(key.scale) || key.scale <= 0 || key.scale > 64 ||
      !std::isfinite(key.device_font_size) || key.device_font_size <= 0 || key.device_font_size > 4096 ||
      !std::isfinite(key.raster_advance) || key.raster_advance < 0 || key.subpixel_x > 3)
    return std::unexpected(invalid_record());
  const auto x = static_cast<std::uint32_t>(upload.atlas_bounds.origin.x);
  const auto y = static_cast<std::uint32_t>(upload.atlas_bounds.origin.y);
  const auto tight = std::uint64_t(upload.width) * upload.height;
  const auto end = std::uint64_t(upload.stride) * (upload.height - 1) + upload.width;
  if (x > width || y > height || upload.width > width - x || upload.height > height - y ||
      tight > std::numeric_limits<std::size_t>::max() || end > std::numeric_limits<std::size_t>::max() ||
      end > upload.alpha.size())
    return std::unexpected(invalid_record());
  const auto entry = std::ranges::find_if(page.entries, [&](const GlyphAtlasEntry& item) {
    return item.key == upload.key && item.page_index == upload.page_index &&
        item.atlas_bounds.origin.x == upload.atlas_bounds.origin.x &&
        item.atlas_bounds.origin.y == upload.atlas_bounds.origin.y &&
        item.atlas_bounds.size.width == upload.atlas_bounds.size.width &&
        item.atlas_bounds.size.height == upload.atlas_bounds.size.height;
  });
  if (entry == page.entries.end()) return std::unexpected(invalid_record());
  return Dx12GlyphRecordLayout{x, y, static_cast<std::size_t>(tight), static_cast<std::size_t>(end)};
}
} // namespace cgpui
