#include "vulkan_frame_diagnostic_snapshot_internal.hpp"

#include <limits>

namespace cgpui {
namespace {

std::size_t saturating_add(
    std::size_t lhs,
    std::size_t rhs,
    bool& saturated) {
  if (rhs > std::numeric_limits<std::size_t>::max() - lhs) {
    saturated = true;
    return std::numeric_limits<std::size_t>::max();
  }
  return lhs + rhs;
}

} // namespace

RendererUploadByteCounts vulkan_merge_upload_byte_counts(
    RendererUploadByteCounts glyph_counts,
    RendererUploadByteCounts image_counts) {
  RendererUploadByteCounts counts{
      .glyph_atlas_byte_count = glyph_counts.glyph_atlas_byte_count,
      .image_byte_count = image_counts.image_byte_count,
      .saturated = glyph_counts.saturated || image_counts.saturated,
  };
  counts.total_byte_count = saturating_add(
      counts.glyph_atlas_byte_count,
      counts.image_byte_count,
      counts.saturated);
  return counts;
}

RendererFrameDiagnosticSnapshot vulkan_build_frame_diagnostic_snapshot(
    std::size_t planned_command_count,
    std::size_t planned_batch_count,
    VulkanFrameDiagnosticResources resources,
    RendererUploadByteCounts upload_bytes,
    RendererFrameTimings timings) {
  const RendererFrameWork planned_work{
      .batch_count = planned_batch_count,
      .command_count = planned_command_count,
      .upload_byte_count = upload_bytes.total_byte_count,
      .draw_count = resources.planned_draws.total_count,
  };
  const RendererFrameWork submitted_work{
      .batch_count = resources.submitted_batch_count,
      .command_count = resources.submitted_command_count,
      .upload_byte_count = upload_bytes.total_byte_count,
      .draw_count = resources.submitted_draws.total_count,
  };
  return RendererFrameDiagnosticSnapshot{
      .work = compare_renderer_frame_work(planned_work, submitted_work),
      .upload_bytes = upload_bytes,
      .planned_draws = resources.planned_draws,
      .submitted_draws = resources.submitted_draws,
      .dropped_resources = std::move(resources.dropped_resources),
      .timings = timings,
  };
}

} // namespace cgpui
