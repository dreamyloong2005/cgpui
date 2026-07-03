#pragma once

#include "cgpui/renderer/glyph_uploads.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace cgpui {

enum class GlyphAtlasTextureResourceStatus {
  created,
  reused,
  dropped,
};

struct GlyphAtlasTextureResourceRecord {
  std::size_t page_index = 0;
  GlyphAtlasImageDescriptor image;
  GlyphAtlasTextureResourceStatus status =
      GlyphAtlasTextureResourceStatus::created;
  std::size_t generation = 0;
};

struct GlyphAtlasTextureResourcePlan {
  std::vector<GlyphAtlasTextureResourceRecord> live_resources;
  std::vector<GlyphAtlasTextureResourceRecord> dropped_resources;
  std::size_t created_count = 0;
  std::size_t reused_count = 0;
  std::size_t dropped_count = 0;
};

struct GlyphAtlasDirtyUploadRange {
  std::size_t page_index = 0;
  std::size_t first_upload_index = 0;
  std::size_t upload_count = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_size = 0;
};

class GlyphAtlasTextureResourceState {
 public:
  [[nodiscard]] std::span<const GlyphAtlasTextureResourceRecord>
  live_resources() const {
    return resources_;
  }

 private:
  friend GlyphAtlasTextureResourcePlan
  vulkan_update_glyph_atlas_texture_resources(
      GlyphAtlasTextureResourceState& state,
      std::span<const GlyphAtlasUploadBatch> upload_batches);
  friend std::vector<GlyphAtlasDirtyUploadRange>
  vulkan_plan_glyph_atlas_dirty_uploads(
      GlyphAtlasTextureResourceState& state,
      std::span<const GlyphAtlasUploadBatch> upload_batches);

  std::vector<GlyphAtlasTextureResourceRecord> resources_;
  std::size_t next_generation_ = 1;
};

GlyphAtlasTextureResourcePlan vulkan_update_glyph_atlas_texture_resources(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches);
std::vector<GlyphAtlasDirtyUploadRange> vulkan_plan_glyph_atlas_dirty_uploads(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches);

} // namespace cgpui
