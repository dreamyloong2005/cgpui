#include "vulkan_report_internal.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>

namespace cgpui {

GlyphAtlasTextureResourcePlan vulkan_update_glyph_atlas_texture_resources(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches) {
  GlyphAtlasTextureResourcePlan plan;
  std::vector<GlyphAtlasTextureResourceRecord> next_resources;
  next_resources.reserve(upload_batches.size());

  for (const GlyphAtlasUploadBatch& batch : upload_batches) {
    auto existing = std::ranges::find_if(
        state.resources_,
        [&](const GlyphAtlasTextureResourceRecord& resource) {
          return resource.page_index == batch.image.page_index;
        });

    GlyphAtlasTextureResourceRecord resource{
        .page_index = batch.image.page_index,
        .image = batch.image,
    };
    resource.image.upload_count = batch.uploads.size();

    if (existing == state.resources_.end()) {
      resource.status = GlyphAtlasTextureResourceStatus::created;
      resource.generation = state.next_generation_++;
      ++plan.created_count;
    } else {
      resource.status = GlyphAtlasTextureResourceStatus::reused;
      resource.generation = existing->generation;
      ++plan.reused_count;
    }

    plan.live_resources.push_back(resource);
    next_resources.push_back(resource);
  }

  for (GlyphAtlasTextureResourceRecord resource : state.resources_) {
    const bool still_live =
        std::ranges::any_of(upload_batches, [&](const auto& batch) {
          return batch.image.page_index == resource.page_index;
        });
    if (!still_live) {
      resource.status = GlyphAtlasTextureResourceStatus::dropped;
      plan.dropped_resources.push_back(resource);
      ++plan.dropped_count;
    }
  }

  state.resources_ = std::move(next_resources);
  return plan;
}

std::vector<GlyphAtlasDirtyUploadRange> vulkan_plan_glyph_atlas_dirty_uploads(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches) {
  std::vector<GlyphAtlasDirtyUploadRange> dirty_ranges;
  for (const GlyphAtlasUploadBatch& batch : upload_batches) {
    auto existing = std::ranges::find_if(
        state.resources_,
        [&](const GlyphAtlasTextureResourceRecord& resource) {
          return resource.page_index == batch.image.page_index;
        });

    std::size_t first_upload_index = 0;
    if (existing != state.resources_.end()) {
      first_upload_index =
          std::min(existing->image.upload_count, batch.uploads.size());
    }
    if (first_upload_index >= batch.uploads.size()) {
      continue;
    }

    const GlyphAtlasUploadRegion& first_upload =
        batch.uploads[first_upload_index];
    const GlyphAtlasUploadRegion& last_upload = batch.uploads.back();
    dirty_ranges.push_back(GlyphAtlasDirtyUploadRange{
        .page_index = batch.image.page_index,
        .first_upload_index = first_upload_index,
        .upload_count = batch.uploads.size() - first_upload_index,
        .byte_offset = first_upload.byte_offset,
        .byte_size =
            last_upload.byte_offset + last_upload.byte_size -
            first_upload.byte_offset,
    });
  }

  (void)vulkan_update_glyph_atlas_texture_resources(state, upload_batches);
  return dirty_ranges;
}

} // namespace cgpui
