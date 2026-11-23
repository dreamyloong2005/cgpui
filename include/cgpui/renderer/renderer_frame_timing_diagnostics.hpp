#pragma once

#include <cstdint>

namespace cgpui {

enum class RendererFrameTimingStage {
  pacing_wait,
  resource_preparation,
  image_acquisition,
  command_recording,
  queue_submission,
  presentation,
};

// Sum of selected nonoverlapping CPU scopes; not GPU, scanout, or end-to-end
// latency. Backends may distribute different work among the same stage names.
struct RendererFrameTimings {
  // Backend-specific frame admission/reuse synchronization, not an exclusive
  // native wait. DX12 includes only the successful begin admission call plus
  // pre-record sync; prior pending probes, event-loop delays and frame holding
  // are excluded. Post-Present synchronization remains in presentation.
  std::uint64_t pacing_wait_nanoseconds = 0;
  std::uint64_t resource_preparation_nanoseconds = 0;
  std::uint64_t image_acquisition_nanoseconds = 0;
  std::uint64_t command_recording_nanoseconds = 0;
  std::uint64_t queue_submission_nanoseconds = 0;
  std::uint64_t presentation_nanoseconds = 0;
  std::uint64_t total_nanoseconds = 0;
  bool saturated = false;
};

void renderer_add_frame_timing(
    RendererFrameTimings& timings,
    RendererFrameTimingStage stage,
    std::uint64_t nanoseconds);

struct RendererFrameTimingDiagnostics {
  std::uint64_t budget_nanoseconds = 0;
  RendererFrameTimings measured_timings;
  std::uint64_t remaining_budget_nanoseconds = 0;
  std::uint64_t over_budget_nanoseconds = 0;

  [[nodiscard]] bool within_budget() const;
};

[[nodiscard]] RendererFrameTimingDiagnostics compare_renderer_frame_timing(
    std::uint64_t budget_nanoseconds,
    RendererFrameTimings measured_timings);

} // namespace cgpui
