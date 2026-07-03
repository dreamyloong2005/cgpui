#include "vulkan_report_internal.hpp"

#include <algorithm>
#include <cstddef>

namespace cgpui {

std::vector<GlyphAtlasUploadBatch> vulkan_plan_glyph_atlas_uploads(
    std::span<const GlyphUploadRecord> upload_records,
    std::span<const GlyphAtlasPage> atlas_pages) {
  std::vector<GlyphAtlasUploadBatch> batches;
  for (const GlyphUploadRecord& upload : upload_records) {
    auto batch = std::ranges::find_if(
        batches,
        [&](const GlyphAtlasUploadBatch& candidate) {
          return candidate.image.page_index == upload.page_index;
        });
    if (batch == batches.end()) {
      Size page_size{.width = 256.0F, .height = 256.0F};
      const auto page = std::ranges::find_if(
          atlas_pages,
          [&](const GlyphAtlasPage& candidate) {
            return candidate.page_index == upload.page_index;
          });
      if (page != atlas_pages.end()) {
        page_size = page->size;
      }
      batches.push_back(GlyphAtlasUploadBatch{
          .image =
              GlyphAtlasImageDescriptor{
                  .page_index = upload.page_index,
                  .size = page_size,
              },
      });
      batch = batches.end() - 1;
    }

    const std::size_t byte_offset = batch->alpha.size();
    batch->alpha.insert(
        batch->alpha.end(),
        upload.alpha.begin(),
        upload.alpha.end());
    batch->uploads.push_back(GlyphAtlasUploadRegion{
        .key = upload.key,
        .page_index = upload.page_index,
        .atlas_bounds = upload.atlas_bounds,
        .width = upload.width,
        .height = upload.height,
        .stride = upload.stride,
        .byte_offset = byte_offset,
        .byte_size = upload.alpha.size(),
    });
    batch->image.upload_count = batch->uploads.size();
  }

  return batches;
}

} // namespace cgpui
