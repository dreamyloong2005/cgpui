#pragma once

#include "cgpui/renderer/glyph_uploads.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace cgpui {

enum class GlyphAtlasProductionResourceStatus {
  created,
  reused,
  dropped,
};

struct GlyphAtlasProductionUploadCommand {
  std::size_t page_index = 0;
  std::size_t first_upload_index = 0;
  std::size_t upload_count = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_size = 0;
  bool stages_alpha_payload = true;
  bool transitions_to_transfer_dst = true;
  bool copies_buffer_to_image = true;
  bool transitions_to_shader_read_only = true;
};

struct GlyphAtlasProductionResourceRecord {
  std::size_t page_index = 0;
  GlyphAtlasImageDescriptor image;
  GlyphAtlasProductionResourceStatus status =
      GlyphAtlasProductionResourceStatus::created;
  std::size_t generation = 0;
  std::size_t uploaded_count = 0;
  bool image_create_ready = true;
  bool memory_allocation_ready = true;
  bool memory_bind_ready = true;
  bool image_view_ready = true;
  bool sampler_ready = true;
  bool descriptor_slot_reserved = true;
  bool descriptor_set_bound = false;
  bool upload_command_path_ready = true;
};

struct GlyphAtlasProductionPlan {
  std::vector<GlyphAtlasProductionResourceRecord> live_resources;
  std::vector<GlyphAtlasProductionResourceRecord> dropped_resources;
  std::vector<GlyphAtlasProductionUploadCommand> upload_commands;
  std::size_t created_count = 0;
  std::size_t reused_count = 0;
  std::size_t dropped_count = 0;
  std::size_t upload_command_count = 0;
  std::size_t dirty_upload_count = 0;
  std::size_t dirty_upload_byte_count = 0;
};

class GlyphAtlasProductionResourceState {
 public:
  [[nodiscard]] std::span<const GlyphAtlasProductionResourceRecord>
  live_resources() const {
    return resources_;
  }

 private:
  friend GlyphAtlasProductionPlan
  vulkan_plan_glyph_atlas_production_resources(
      GlyphAtlasProductionResourceState& state,
      std::span<const GlyphAtlasUploadBatch> upload_batches);

  std::vector<GlyphAtlasProductionResourceRecord> resources_;
  std::size_t next_generation_ = 1;
};

GlyphAtlasProductionPlan vulkan_plan_glyph_atlas_production_resources(
    GlyphAtlasProductionResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches);

} // namespace cgpui
