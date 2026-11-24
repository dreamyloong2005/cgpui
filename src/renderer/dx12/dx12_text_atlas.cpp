#include "dx12_text_cache_internal.hpp"
#include "dx12_glyph_record_validation_internal.hpp"

#include <cmath>
#include <cstring>

namespace cgpui {
namespace {
Error invalid_atlas() {
  return {.code = ErrorCode::invalid_argument,
          .message = "DX12 glyph atlas has invalid dimensions or upload bounds"};
}
bool integer_coordinate(float value) {
  return std::isfinite(value) && value >= 0 &&
      value <= D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION && std::trunc(value) == value;
}
void copy_upload(const GlyphUploadRecord& upload, const Dx12GlyphRecordLayout& layout,
                 Dx12GlyphPage& page) noexcept {
  for (std::uint32_t row = 0; row < upload.height; ++row)
    std::memcpy(page.alpha.data() + std::size_t(layout.y + row) * page.width + layout.x,
                upload.alpha.data() + std::size_t(row) * upload.stride, upload.width);
  page.dirty = true;
}
} // namespace

Result<void> dx12_update_glyph_pages(Dx12State& state, Dx12TextCache& cache) {
  Dx12GlyphLineageSweep sweep(cache.lineage);
  const auto pages = cache.glyphs.atlas_pages();
  while (cache.pages.size() < pages.size()) {
    const auto& source = pages[cache.pages.size()];
    if (source.page_index != cache.pages.size() || !integer_coordinate(source.size.width) ||
        !integer_coordinate(source.size.height) || source.size.width < 1 || source.size.height < 1)
      return std::unexpected(invalid_atlas());
    const auto width = static_cast<std::uint32_t>(source.size.width);
    const auto height = static_cast<std::uint32_t>(source.size.height);
    if (auto family = cache.lineage.ensure_page(source.page_index, width, height); !family) return family;
    cache.pages.push_back({.width = width, .height = height,
        .alpha = std::vector<std::uint8_t>(std::size_t(width) * height)});
  }
  const auto uploads = cache.glyphs.upload_records();
  if (cache.processed_uploads > uploads.size() || cache.pages.size() != pages.size())
    return std::unexpected(invalid_atlas());
  while (cache.processed_uploads < uploads.size()) {
    const auto& upload = uploads[cache.processed_uploads];
    if (upload.page_index >= cache.pages.size()) return std::unexpected(invalid_atlas());
    auto& page = cache.pages[upload.page_index];
    auto layout = dx12_validate_glyph_record(upload, pages[upload.page_index], page.width, page.height);
    if (!layout) return std::unexpected(layout.error());
    if (auto prepared = cache.lineage.prepare_record(upload.page_index, cache.processed_uploads); !prepared)
      return prepared;
    // Pixel copy and both progress counters below are nonallocating.
    copy_upload(upload, *layout, page);
    cache.lineage.commit_record(upload.page_index, cache.processed_uploads);
    ++cache.processed_uploads;
  }
  for (std::size_t index = 0; index < cache.pages.size(); ++index) {
    auto& page = cache.pages[index];
    if (!page.dirty) continue;
    auto snapshot = dx12_create_texture(state, page.width, page.height, page.width,
                                        DXGI_FORMAT_R8_UNORM, page.alpha);
    if (!snapshot) return std::unexpected(snapshot.error());
    if (auto sealed = cache.lineage.seal_snapshot(index, *snapshot); !sealed) return sealed;
    page.texture = std::move(*snapshot);
    page.dirty = false;
    cache.lineage.prune_expired(); // Old page owner has been released before sweeping.
  }
  return {};
}
} // namespace cgpui
