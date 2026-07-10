#pragma once

#include "cgpui/renderer/renderer_frame_diagnostic_snapshot.hpp"
#include "vulkan_frame_draw_order_internal.hpp"
#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"

#include <chrono>

namespace cgpui {

struct VulkanFrameDiagnosticResources {
  std::vector<RendererFrameResource> planned_resources;
  std::vector<RendererFrameResource> submitted_resources;
  RendererDrawCounts planned_draws;
  RendererDrawCounts submitted_draws;
  RendererDroppedResourceDiagnostics dropped_resources;
  std::size_t submitted_command_count = 0;
  std::size_t submitted_batch_count = 0;
};

[[nodiscard]] VulkanFrameDiagnosticResources
vulkan_build_frame_diagnostic_resources(
    std::span<const VulkanFrameDrawOrderEntry> draw_order,
    std::span<const VulkanRoundedRectDrawRange> solid_draws,
    std::span<const VulkanRoundedRectDrawRange> rounded_draws,
    std::span<const VulkanGlyphAtlasDrawBinding> text_bindings,
    std::span<const ImageDraw> image_draws,
    std::span<const RendererCommandBatch> command_batches);

[[nodiscard]] RendererFrameDiagnosticSnapshot
vulkan_build_frame_diagnostic_snapshot(
    std::size_t planned_command_count,
    std::size_t planned_batch_count,
    VulkanFrameDiagnosticResources resources,
    RendererUploadByteCounts upload_bytes,
    RendererFrameTimings timings);

[[nodiscard]] RendererUploadByteCounts vulkan_merge_upload_byte_counts(
    RendererUploadByteCounts glyph_counts,
    RendererUploadByteCounts image_counts);

class VulkanFrameDiagnosticTimer {
 public:
  VulkanFrameDiagnosticTimer();
  void finish_stage(RendererFrameTimingStage stage);
  [[nodiscard]] RendererFrameTimings timings() const;

 private:
  using Clock = std::chrono::steady_clock;
  Clock::time_point stage_start_;
  RendererFrameTimings timings_;
};

} // namespace cgpui
