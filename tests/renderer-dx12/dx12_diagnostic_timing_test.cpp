#include "../../src/renderer/dx12/dx12_frame_diagnostics_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace cgpui;

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
auto fields(const RendererFrameTimings& timing) {
  return std::array<std::uint64_t, 8>{timing.pacing_wait_nanoseconds,
      timing.resource_preparation_nanoseconds, timing.image_acquisition_nanoseconds,
      timing.command_recording_nanoseconds, timing.queue_submission_nanoseconds,
      timing.presentation_nanoseconds, timing.total_nanoseconds, timing.saturated ? 1U : 0U};
}
void exact_ledger_sum(const RendererFrameTimings& timing) {
  require(!timing.saturated && timing.total_nanoseconds == timing.pacing_wait_nanoseconds +
      timing.resource_preparation_nanoseconds + timing.image_acquisition_nanoseconds +
      timing.command_recording_nanoseconds + timing.queue_submission_nanoseconds +
      timing.presentation_nanoseconds, "selected CPU stage intervals do not sum to total");
}
void publish(Dx12State& state, std::uint64_t admission_call_ns) {
  Dx12FrameDiagnostics candidate(state, {}, 0, admission_call_ns);
  candidate.publish();
  const auto once = fields(state.diagnostics->timings);
  candidate.publish();
  require(fields(state.diagnostics->timings) == once, "repeat publication added the seed twice");
}
void scoped_seed_handoff() {
  Dx12State state; // CPU ledger only: no device, swapchain, native wait or frame admission.
  {
    Dx12FrameDiagnostics candidate(state, {}, 0, 17);
    require(state.active_diagnostics == &candidate && !state.diagnostics,
        "constructor exposed an uncompleted timing candidate");
  }
  require(!state.active_diagnostics && !state.diagnostics,
      "unpublished initial timing candidate became a completed snapshot");
  publish(state, 17);
  require(state.diagnostics && state.diagnostics->timings.pacing_wait_nanoseconds == 17,
      "known 17 ns admission interval was omitted or double-counted");
  exact_ledger_sum(state.diagnostics->timings);
  const auto previous = fields(state.diagnostics->timings);
  {
    Dx12FrameDiagnostics abandoned(state, {}, 0, 999);
  }
  require(!state.active_diagnostics && fields(state.diagnostics->timings) == previous,
      "unpublished later candidate overwrote the last successful timings");
  publish(state, 0);
  require(state.diagnostics->timings.pacing_wait_nanoseconds == 0,
      "zero-seed frame inherited an earlier admission interval");
  exact_ledger_sum(state.diagnostics->timings);
  publish(state, 5);
  require(state.diagnostics->timings.pacing_wait_nanoseconds == 5,
      "new successful candidate reused a stale frame's scalar");
  exact_ledger_sum(state.diagnostics->timings);
  // Same public saturating addition used for the later pre-record sync lap.
  auto multiple = state.diagnostics->timings;
  const auto total = multiple.total_nanoseconds;
  renderer_add_frame_timing(multiple, RendererFrameTimingStage::pacing_wait, 12);
  require(multiple.pacing_wait_nanoseconds == 17 && multiple.total_nanoseconds == total + 12,
      "separate admission and synchronization contributions did not add exactly once");
  exact_ledger_sum(multiple);
}
void saturating_seed() {
  Dx12State state;
  const auto maximum = std::numeric_limits<std::uint64_t>::max();
  publish(state, maximum);
  auto timing = state.diagnostics->timings;
  require(timing.pacing_wait_nanoseconds == maximum && timing.total_nanoseconds == maximum,
      "maximum admission interval wrapped or was omitted from the total");
  // Planning may measure zero on a coarse clock; force overflow with a known
  // second contribution instead of assuming any measured interval is positive.
  renderer_add_frame_timing(timing, RendererFrameTimingStage::pacing_wait, 1);
  require(timing.pacing_wait_nanoseconds == maximum && timing.total_nanoseconds == maximum &&
      timing.saturated, "timing category/total did not saturate on an explicit extra contribution");
}
} // namespace

int main() {
  try {
    scoped_seed_handoff();
    saturating_seed();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
