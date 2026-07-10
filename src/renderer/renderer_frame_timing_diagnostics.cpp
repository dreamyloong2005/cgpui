#include "cgpui/renderer/renderer_frame_timing_diagnostics.hpp"

#include <limits>

namespace cgpui {
namespace {

std::uint64_t saturating_add(
    std::uint64_t value,
    std::uint64_t increment,
    bool& saturated) {
  if (increment > std::numeric_limits<std::uint64_t>::max() - value) {
    saturated = true;
    return std::numeric_limits<std::uint64_t>::max();
  }
  return value + increment;
}

std::uint64_t& frame_stage_timing(
    RendererFrameTimings& timings,
    RendererFrameTimingStage stage) {
  switch (stage) {
    case RendererFrameTimingStage::pacing_wait:
      return timings.pacing_wait_nanoseconds;
    case RendererFrameTimingStage::resource_preparation:
      return timings.resource_preparation_nanoseconds;
    case RendererFrameTimingStage::image_acquisition:
      return timings.image_acquisition_nanoseconds;
    case RendererFrameTimingStage::command_recording:
      return timings.command_recording_nanoseconds;
    case RendererFrameTimingStage::queue_submission:
      return timings.queue_submission_nanoseconds;
    case RendererFrameTimingStage::presentation:
      return timings.presentation_nanoseconds;
  }
  return timings.presentation_nanoseconds;
}

} // namespace

void renderer_add_frame_timing(
    RendererFrameTimings& timings,
    RendererFrameTimingStage stage,
    std::uint64_t nanoseconds) {
  std::uint64_t& stage_timing = frame_stage_timing(timings, stage);
  stage_timing = saturating_add(stage_timing, nanoseconds, timings.saturated);
  timings.total_nanoseconds = saturating_add(
      timings.total_nanoseconds, nanoseconds, timings.saturated);
}

bool RendererFrameTimingDiagnostics::within_budget() const {
  return over_budget_nanoseconds == 0;
}

RendererFrameTimingDiagnostics compare_renderer_frame_timing(
    std::uint64_t budget_nanoseconds,
    RendererFrameTimings measured_timings) {
  const std::uint64_t measured_nanoseconds =
      measured_timings.total_nanoseconds;
  return RendererFrameTimingDiagnostics{
      .budget_nanoseconds = budget_nanoseconds,
      .measured_timings = measured_timings,
      .remaining_budget_nanoseconds =
          budget_nanoseconds > measured_nanoseconds
              ? budget_nanoseconds - measured_nanoseconds
              : 0,
      .over_budget_nanoseconds =
          measured_nanoseconds > budget_nanoseconds
              ? measured_nanoseconds - budget_nanoseconds
              : 0,
  };
}

} // namespace cgpui
