#include "ui_internal.hpp"

namespace cgpui {

void apply_runtime_renderer_frame_diagnostics(
    FrameStatistics& statistics,
    const RendererFrameDiagnosticSnapshot* snapshot) {
  if (snapshot == nullptr) {
    statistics.renderer_frame.reset();
    statistics.frame_time_ms = 0.0;
    return;
  }
  statistics.renderer_frame = RendererFrameStatistics{
      .work = snapshot->work,
      .upload_bytes = snapshot->upload_bytes,
      .planned_draws = snapshot->planned_draws,
      .submitted_draws = snapshot->submitted_draws,
      .dropped_draws = snapshot->dropped_resources.counts,
      .dropped_resource_count = snapshot->dropped_resources.resources.size(),
      .unsupported_primitive_count =
          snapshot->dropped_resources.unsupported_primitive_count,
      .missing_submission_resource_count =
          snapshot->dropped_resources.missing_submission_resource_count,
      .timings = snapshot->timings,
  };
  statistics.frame_time_ms =
      static_cast<double>(snapshot->timings.total_nanoseconds) / 1'000'000.0;
}

std::optional<RendererFrameDiagnosticSnapshot>
runtime_renderer_frame_diagnostic_snapshot(const Renderer& renderer) {
  const RendererFrameDiagnosticSnapshot* snapshot =
      renderer.last_frame_diagnostic_snapshot();
  return snapshot == nullptr
             ? std::nullopt
             : std::optional<RendererFrameDiagnosticSnapshot>{*snapshot};
}

} // namespace cgpui
