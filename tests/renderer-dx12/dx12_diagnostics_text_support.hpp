#pragma once

#include "dx12_diagnostics_support.hpp"
#include "cgpui/renderer/planning/glyph_geometry.hpp"

namespace cgpui_test {
inline std::size_t diagnostic_atlas_bytes(const cgpui::GlyphCache& cache) {
  std::size_t bytes = 0;
  for (const auto& page : cache.atlas_pages())
    bytes += std::size_t(page.size.width) * std::size_t(page.size.height); // alpha8, tightly packed
  return bytes;
}
inline std::size_t diagnostic_atlas_runs(std::span<const cgpui::TexturedGlyphQuad> quads) {
  std::size_t runs = 0, previous = 0;
  for (const auto& quad : quads) {
    if (!runs || quad.page_index != previous) ++runs;
    previous = quad.page_index;
  }
  return runs;
}
inline bool diagnostic_glyph_pixels(const cgpui::RendererFramePixels* pixels,
    std::span<const cgpui::TexturedGlyphQuad> quads, const cgpui::GlyphCache& reference) {
  if (!pixels || !pixels->valid() || quads.empty()) return false;
  // Only native font coverage is shared with the public CPU oracle. Each glyph
  // must have visible white ink over literal opaque black, never an equal blank.
  for (const auto& quad : quads) {
    const cgpui::GlyphUploadRecord* upload = nullptr;
    // Upload keys canonicalize repeated clusters; quads retain original paint
    // metadata. Their shared allocation identity is the page and atlas bounds.
    for (const auto& candidate : reference.upload_records())
      if (candidate.page_index == quad.page_index &&
          candidate.atlas_bounds.origin.x == quad.atlas_bounds.origin.x &&
          candidate.atlas_bounds.origin.y == quad.atlas_bounds.origin.y &&
          candidate.atlas_bounds.size.width == quad.atlas_bounds.size.width &&
          candidate.atlas_bounds.size.height == quad.atlas_bounds.size.height) { upload = &candidate; break; }
    if (!upload) return false;
    unsigned ink = 0;
    for (unsigned y = 0; y < upload->height && ink < 8; ++y) {
      for (unsigned x = 0; x < upload->width && ink < 8; ++x) {
        const auto alpha = upload->alpha[std::size_t(y) * upload->stride + x];
        if (alpha <= 16) continue;
        const auto px = unsigned(std::lround(quad.device_bounds.origin.x)) + x;
        const auto py = unsigned(std::lround(quad.device_bounds.origin.y)) + y;
        if (!dx12_pixel(pixels, px, py, {alpha, alpha, alpha, 255})) return false;
        ++ink;
      }
    }
    if (ink < 8) return false;
  }
  return true;
}
} // namespace cgpui_test
