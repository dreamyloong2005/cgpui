#pragma once
#include "../../src/renderer/dx12/dx12_glyph_upload_plan_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_internal.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace cgpui_test {
inline void glyph_require(bool value, const char* reason) {
  if (!value) throw std::runtime_error(reason);
}
inline cgpui::Dx12TextureHandle glyph_cpu_snapshot(unsigned width = 16, unsigned height = 16) {
  auto result = std::make_shared<cgpui::Dx12Texture>();
  result->width = width; result->height = height; result->stride = width;
  result->format = DXGI_FORMAT_R8_UNORM;
  result->pixels.resize(std::size_t(width) * height);
  return result;
}
inline cgpui::GlyphUploadRecord glyph_cpu_record(
    std::size_t id, std::size_t page, unsigned x, unsigned y, unsigned width = 3,
    unsigned height = 2, unsigned stride = 5) {
  cgpui::GlyphUploadRecord result{.key = {.font_family = "fixture", .glyph_id = static_cast<unsigned>(id)},
      .page_index = page, .atlas_bounds = {{float(x), float(y)}, {float(width), float(height)}},
      .width = width, .height = height, .stride = stride,
      .alpha = std::vector<std::uint8_t>(std::size_t(stride) * (height - 1) + width, 0xe1)};
  for (unsigned row = 0; row < height; ++row) for (unsigned col = 0; col < width; ++col)
    result.alpha[std::size_t(row) * stride + col] = static_cast<std::uint8_t>(id * 17 + row * 9 + col);
  return result;
}
struct GlyphCpuHistory {
  cgpui::Dx12GlyphLineage lineage;
  std::vector<cgpui::GlyphUploadRecord> records;
  std::vector<cgpui::GlyphAtlasPage> pages{{.page_index = 0, .size = {16, 16}},
                                        {.page_index = 1, .size = {16, 16}}};
  cgpui::Dx12TextureHandle snapshot = glyph_cpu_snapshot();
  cgpui::Dx12GlyphSnapshot sealed;
  GlyphCpuHistory() {
    glyph_require(bool(lineage.ensure_page(0, 16, 16)) && bool(lineage.ensure_page(1, 16, 16)), "families");
    for (std::size_t g = 0; g < 9; ++g) {
      const std::size_t page = (g == 0 || g == 3 || g == 7 || g == 8) ? 0 : 1;
      auto record = g == 8 ? glyph_cpu_record(g, page, 7, 8, 5, 3, 8)
                           : glyph_cpu_record(g, page, 1, 2);
      pages[page].entries.push_back({.key = record.key, .page_index = page, .atlas_bounds = record.atlas_bounds});
      records.push_back(std::move(record));
      glyph_require(bool(lineage.prepare_record(page, g)), "prepare history");
      lineage.commit_record(page, g);
    }
    glyph_require(bool(lineage.seal_snapshot(0, snapshot)), "seal target");
    sealed = *lineage.find_snapshot(snapshot);
  }
};
} // namespace cgpui_test
