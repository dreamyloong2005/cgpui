#pragma once

#include "cgpui/renderer/glyph_atlas_types.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace cgpui {

struct GlyphUploadRecord {
  GlyphAtlasKey key;
  std::size_t page_index = 0;
  Rect atlas_bounds;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t stride = 0;
  std::vector<std::uint8_t> alpha;
};

struct GlyphAtlasImageDescriptor {
  std::size_t page_index = 0;
  Size size{.width = 256.0F, .height = 256.0F};
  GlyphAtlasImageFormat format = GlyphAtlasImageFormat::alpha8_unorm;
  std::size_t upload_count = 0;
};

struct GlyphAtlasUploadRegion {
  GlyphAtlasKey key;
  std::size_t page_index = 0;
  Rect atlas_bounds;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t stride = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_size = 0;
};

struct GlyphAtlasUploadBatch {
  GlyphAtlasImageDescriptor image;
  std::vector<GlyphAtlasUploadRegion> uploads;
  std::vector<std::uint8_t> alpha;
};

std::vector<GlyphAtlasUploadBatch> vulkan_plan_glyph_atlas_uploads(
    std::span<const GlyphUploadRecord> upload_records,
    std::span<const GlyphAtlasPage> atlas_pages);

} // namespace cgpui
