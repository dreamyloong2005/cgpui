#include "vulkan_frame_diagnostic_snapshot_internal.hpp"

namespace cgpui {

VulkanFrameDiagnosticTimer::VulkanFrameDiagnosticTimer()
    : stage_start_(Clock::now()) {}

void VulkanFrameDiagnosticTimer::finish_stage(RendererFrameTimingStage stage) {
  const Clock::time_point finished = Clock::now();
  const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
      finished - stage_start_);
  renderer_add_frame_timing(
      timings_, stage, static_cast<std::uint64_t>(elapsed.count()));
  stage_start_ = finished;
}

RendererFrameTimings VulkanFrameDiagnosticTimer::timings() const {
  return timings_;
}

} // namespace cgpui
