#include "cgpui/prelude.hpp"
#include "../ui/window_runtime_test_support.hpp"

#include <cstdint>
#include <vector>

namespace {

struct PacingState {
  cgpui::WindowRuntime* runtime = nullptr;
  RuntimeFixture* fixture = nullptr;
  cgpui::AnimationHandle first;
  cgpui::AnimationHandle second;
  cgpui::AnimationHandle slow;
  cgpui::AnimationHandle mutation_first;
  cgpui::AnimationHandle mutation_second;
  cgpui::AnimationHandle spawned;
  std::vector<std::uint64_t> first_ticks;
  std::vector<std::uint64_t> second_ticks;
  std::vector<std::uint64_t> slow_ticks;
  int mutation_first_ticks = 0;
  int mutation_second_ticks = 0;
  int spawned_ticks = 0;
  int failure = 0;
};

PacingState* pacing_state = nullptr;

void run_pacing_scenario() {
  auto& state = *pacing_state;
  auto& runtime = *state.runtime;
  auto& fixture = *state.fixture;
  state.first = runtime.start_animation(
      cgpui::AnimationOptions{.duration_ms = 64, .tick_interval_ms = 16},
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::AnimationSnapshot& snapshot) {
        state.first_ticks.push_back(snapshot.elapsed_ms);
      });
  state.second = runtime.start_animation(
      cgpui::AnimationOptions{.duration_ms = 64, .tick_interval_ms = 16},
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::AnimationSnapshot& snapshot) {
        state.second_ticks.push_back(snapshot.elapsed_ms);
      });
  state.slow = runtime.start_animation(
      cgpui::AnimationOptions{.duration_ms = 64, .tick_interval_ms = 32},
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::AnimationSnapshot& snapshot) {
        state.slow_ticks.push_back(snapshot.elapsed_ms);
      });

  auto pacing = runtime.diagnostics_snapshot().animation_frame_pacing;
  if (pacing.pending_animation_count != 3 || !pacing.wakeup_scheduled ||
      pacing.next_frame_deadline_ms != 16 ||
      pacing.scheduled_wakeup_count != 1 ||
      pacing.coalesced_request_count != 2 ||
      fixture.app.delayed_wakeup_delays !=
          std::vector<std::uint64_t>({16})) {
    state.failure = 1;
    return;
  }

  fixture.app.advance_monotonic_time(20);
  fixture.app.dispatch_delayed_wakeup();
  pacing = runtime.diagnostics_snapshot().animation_frame_pacing;
  if (state.first_ticks != std::vector<std::uint64_t>({20}) ||
      state.second_ticks != std::vector<std::uint64_t>({20}) ||
      !state.slow_ticks.empty() || pacing.delivered_frame_count != 1 ||
      pacing.late_frame_count != 1 || pacing.next_frame_deadline_ms != 32 ||
      pacing.last_scheduled_delay_ms != 12) {
    state.failure = 2;
    return;
  }

  fixture.app.advance_monotonic_time(12);
  fixture.app.dispatch_delayed_wakeup();
  pacing = runtime.diagnostics_snapshot().animation_frame_pacing;
  if (state.first_ticks != std::vector<std::uint64_t>({20, 32}) ||
      state.second_ticks != std::vector<std::uint64_t>({20, 32}) ||
      state.slow_ticks != std::vector<std::uint64_t>({32}) ||
      pacing.delivered_frame_count != 2 ||
      pacing.next_frame_deadline_ms != 48) {
    state.failure = 3;
    return;
  }

  if (!state.first.cancel() || !state.second.cancel() ||
      !state.slow.cancel()) {
    state.failure = 4;
    return;
  }
  pacing = runtime.diagnostics_snapshot().animation_frame_pacing;
  if (pacing.pending_animation_count != 0 || pacing.wakeup_scheduled ||
      pacing.next_frame_deadline_ms != 0) {
    state.failure = 5;
    return;
  }

  state.mutation_first = runtime.start_animation(
      cgpui::AnimationOptions{.duration_ms = 32, .tick_interval_ms = 16},
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::AnimationSnapshot&) {
        state.mutation_first_ticks += 1;
        if (state.mutation_first_ticks != 1) return;
        state.spawned = runtime.start_animation(
            cgpui::AnimationOptions{
                .duration_ms = 16,
                .tick_interval_ms = 16,
            },
            [&](const cgpui::WindowRuntimeContext&,
                const cgpui::AnimationSnapshot&) {
              state.spawned_ticks += 1;
            });
        (void)state.mutation_second.cancel();
      });
  state.mutation_second = runtime.start_animation(
      cgpui::AnimationOptions{.duration_ms = 32, .tick_interval_ms = 16},
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::AnimationSnapshot&) {
        state.mutation_second_ticks += 1;
      });
  fixture.app.advance_monotonic_time(16);
  fixture.app.dispatch_delayed_wakeup();
  pacing = runtime.diagnostics_snapshot().animation_frame_pacing;
  if (state.mutation_first_ticks != 1 || state.mutation_second_ticks != 0 ||
      state.spawned_ticks != 0 || !state.mutation_second.cancelled() ||
      pacing.pending_animation_count != 2 ||
      pacing.next_frame_deadline_ms != 64) {
    state.failure = 6;
    return;
  }
  fixture.app.advance_monotonic_time(16);
  fixture.app.dispatch_delayed_wakeup();
  pacing = runtime.diagnostics_snapshot().animation_frame_pacing;
  if (state.mutation_first_ticks != 2 || state.mutation_second_ticks != 0 ||
      state.spawned_ticks != 1 || pacing.pending_animation_count != 0 ||
      pacing.wakeup_scheduled) {
    state.failure = 7;
  }
}

} // namespace

int main() {
  RuntimeFixture fixture;
  fixture.app.on_run = &run_pacing_scenario;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  PacingState state{.runtime = &runtime, .fixture = &fixture};
  pacing_state = &state;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  pacing_state = nullptr;
  if (result != 0) return 10;
  return state.failure;
}
