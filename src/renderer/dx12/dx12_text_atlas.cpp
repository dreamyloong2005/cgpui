#include "dx12_text_cache_internal.hpp"

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

Result<void> copy_upload(const GlyphUploadRecord& upload, Dx12GlyphPage& page) {
  const auto origin = upload.atlas_bounds.origin;
  if (!integer_coordinate(origin.x) || !integer_coordinate(origin.y) ||
      !upload.width || !upload.height || upload.stride < upload.width)
    return std::unexpected(invalid_atlas());
  const auto x = static_cast<std::uint32_t>(origin.x);
  const auto y = static_cast<std::uint32_t>(origin.y);
  if (x > page.width || y > page.height || upload.width > page.width - x ||
      upload.height > page.height - y || upload.alpha.size() <
          std::size_t(upload.stride) * (upload.height - 1) + upload.width)
    return std::unexpected(invalid_atlas());
  for (std::uint32_t row = 0; row < upload.height; ++row) {
    std::memcpy(page.alpha.data() + std::size_t(y + row) * page.width + x,
                upload.alpha.data() + std::size_t(row) * upload.stride, upload.width);
  }
  page.dirty = true;
  return {};
}
} // namespace

Result<void> dx12_update_glyph_pages(Dx12State& state, Dx12TextCache& cache) {
  const auto pages = cache.glyphs.atlas_pages();
  while (cache.pages.size() < pages.size()) {
    const auto& source = pages[cache.pages.size()];
    if (source.page_index != cache.pages.size() || !integer_coordinate(source.size.width) ||
        !integer_coordinate(source.size.height) || source.size.width < 1 || source.size.height < 1)
      return std::unexpected(invalid_atlas());
    const auto width = static_cast<std::uint32_t>(source.size.width);
    const auto height = static_cast<std::uint32_t>(source.size.height);
    cache.pages.push_back({.width = width, .height = height,
        .alpha = std::vector<std::uint8_t>(std::size_t(width) * height)});
  }
  const auto uploads = cache.glyphs.upload_records();
  for (; cache.processed_uploads < uploads.size(); ++cache.processed_uploads) {
    const auto& upload = uploads[cache.processed_uploads];
    if (upload.page_index >= cache.pages.size()) return std::unexpected(invalid_atlas());
    if (auto copied = copy_upload(upload, cache.pages[upload.page_index]); !copied) return copied;
  }
  for (auto& page : cache.pages) {
    if (!page.dirty) continue;
    auto snapshot = dx12_create_texture(state, page.width, page.height, page.width,
                                        DXGI_FORMAT_R8_UNORM, page.alpha);
    if (!snapshot) return std::unexpected(snapshot.error());
    page.texture = std::move(*snapshot);
    page.dirty = false;
  }
  return {};
}
} // namespace cgpui
