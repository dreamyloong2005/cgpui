#include "cgpui/renderer/glyph_atlas_production.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>

namespace cgpui {
namespace {

[[nodiscard]] std::vector<GlyphAtlasProductionResourceRecord>::iterator
find_live_resource(
    std::vector<GlyphAtlasProductionResourceRecord>& resources,
    std::size_t page_index) {
  return std::ranges::find_if(
      resources,
      [&](const GlyphAtlasProductionResourceRecord& resource) {
        return resource.page_index == page_index;
      });
}

void append_dirty_upload_command(
    GlyphAtlasProductionPlan& plan,
    const GlyphAtlasUploadBatch& batch,
    std::size_t first_upload_index) {
  if (first_upload_index >= batch.uploads.size()) {
    return;
  }

  const GlyphAtlasUploadRegion& first_upload =
      batch.uploads[first_upload_index];
  const GlyphAtlasUploadRegion& last_upload = batch.uploads.back();
  const std::size_t byte_size =
      last_upload.byte_offset + last_upload.byte_size - first_upload.byte_offset;
  plan.upload_commands.push_back(GlyphAtlasProductionUploadCommand{
      .page_index = batch.image.page_index,
      .first_upload_index = first_upload_index,
      .upload_count = batch.uploads.size() - first_upload_index,
      .byte_offset = first_upload.byte_offset,
      .byte_size = byte_size,
  });
  plan.upload_command_count = plan.upload_commands.size();
  plan.dirty_upload_count += batch.uploads.size() - first_upload_index;
  plan.dirty_upload_byte_count += byte_size;
}

} // namespace

GlyphAtlasProductionPlan vulkan_plan_glyph_atlas_production_resources(
    GlyphAtlasProductionResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches) {
  GlyphAtlasProductionPlan plan;
  std::vector<GlyphAtlasProductionResourceRecord> next_resources;
  next_resources.reserve(upload_batches.size());

  for (const GlyphAtlasUploadBatch& batch : upload_batches) {
    auto existing = find_live_resource(state.resources_, batch.image.page_index);
    GlyphAtlasProductionResourceRecord resource{
        .page_index = batch.image.page_index,
        .image = batch.image,
    };
    resource.image.upload_count = batch.uploads.size();
    resource.uploaded_count = batch.uploads.size();

    std::size_t first_dirty_upload_index = 0;
    if (existing == state.resources_.end()) {
      resource.status = GlyphAtlasProductionResourceStatus::created;
      resource.generation = state.next_generation_++;
      ++plan.created_count;
    } else {
      resource.status = GlyphAtlasProductionResourceStatus::reused;
      resource.generation = existing->generation;
      first_dirty_upload_index =
          std::min(existing->uploaded_count, batch.uploads.size());
      ++plan.reused_count;
    }

    append_dirty_upload_command(plan, batch, first_dirty_upload_index);
    plan.live_resources.push_back(resource);
    next_resources.push_back(resource);
  }

  for (GlyphAtlasProductionResourceRecord resource : state.resources_) {
    const bool still_live =
        std::ranges::any_of(upload_batches, [&](const auto& batch) {
          return batch.image.page_index == resource.page_index;
        });
    if (!still_live) {
      resource.status = GlyphAtlasProductionResourceStatus::dropped;
      plan.dropped_resources.push_back(resource);
      ++plan.dropped_count;
    }
  }

  state.resources_ = std::move(next_resources);
  return plan;
}

} // namespace cgpui
