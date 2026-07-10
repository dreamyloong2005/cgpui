#include "cgpui/renderer/renderer_frame_diagnostics.hpp"

#include <limits>

namespace cgpui {
namespace {

std::size_t saturating_add(
    std::size_t value,
    std::size_t increment,
    bool& saturated) {
  if (increment > std::numeric_limits<std::size_t>::max() - value) {
    saturated = true;
    return std::numeric_limits<std::size_t>::max();
  }
  return value + increment;
}

void add_upload_bytes(
    std::size_t byte_size,
    std::size_t& category_byte_count,
    RendererUploadByteCounts& counts) {
  category_byte_count =
      saturating_add(category_byte_count, byte_size, counts.saturated);
  counts.total_byte_count =
      saturating_add(counts.total_byte_count, byte_size, counts.saturated);
}

} // namespace

RendererUploadByteCounts renderer_upload_byte_counts(
    std::span<const GlyphAtlasUploadBatch> glyph_uploads,
    std::span<const ImageUploadBatch> image_uploads) {
  RendererUploadByteCounts counts;
  for (const GlyphAtlasUploadBatch& batch : glyph_uploads) {
    for (const GlyphAtlasUploadRegion& upload : batch.uploads) {
      add_upload_bytes(
          upload.byte_size,
          counts.glyph_atlas_byte_count,
          counts);
    }
  }
  for (const ImageUploadBatch& batch : image_uploads) {
    for (const ImageUploadRegion& upload : batch.uploads) {
      add_upload_bytes(upload.byte_size, counts.image_byte_count, counts);
    }
  }
  return counts;
}

} // namespace cgpui
