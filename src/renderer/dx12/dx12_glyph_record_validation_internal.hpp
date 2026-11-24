#pragma once
#include "cgpui/core/error.hpp"
#include "cgpui/renderer/glyph_uploads.hpp"
#include <cstddef>
#include <cstdint>

namespace cgpui {
struct Dx12GlyphRecordLayout {
  std::uint32_t x = 0, y = 0;
  std::size_t tight_bytes = 0, last_row_end = 0;
};
Result<Dx12GlyphRecordLayout> dx12_validate_glyph_record(
    const GlyphUploadRecord&, const GlyphAtlasPage&, std::uint32_t width, std::uint32_t height);
} // namespace cgpui
