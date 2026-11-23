#include "dx12_runtime_pacing_test_support.hpp"
#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !pacing_begin_debug_enable()) return dx12_fail(1, "backoff fixture unavailable");
  auto application = create_platform_application();
  if (!application) return dx12_fail(2, application.error().message.c_str());
  Dx12RuntimePacingFixture fixture{.application = **application};
  unsigned phase = 0, ticks_before = 0;
  std::array<unsigned, 2> polls_before{};
  auto phase_started = Dx12RuntimePacingFixture::Clock::now();
  fixture.on_tick = [&](const WindowRuntimeContext&) {
    if (fixture.factories != 2) return;
    const auto now = Dx12RuntimePacingFixture::Clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - phase_started).count();
    if (phase == 0) {
      if (fixture.windows[0].pending < 2 || fixture.windows[1].pending < 2) return;
      phase_started = now;
      phase = 1;
    } else if (phase == 1 && elapsed >= 350) {
      if (!fixture.require(fixture.pending_without_success(), "sustained pending fabricated success")) return;
      polls_before = {fixture.windows[0].api.polls, fixture.windows[1].api.polls};
      ticks_before = fixture.user_ticks;
      phase_started = now;
      phase = 2;
    } else if (phase == 2 && elapsed >= 192) {
      if (!fixture.require(fixture.pending_without_success() && fixture.user_ticks > ticks_before + 1,
          "quiet pending windows blocked independent user timers")) return;
      // A measured quiet interval, not an exact refresh/CPU guarantee. Allow
      // boundary/WM_PAINT slack but reject continued short-delay polling.
      const auto maximum = static_cast<unsigned>(elapsed / 64) + 3;
      for (unsigned index = 0; index != 2; ++index) {
        const auto polls = fixture.windows[index].api.polls - polls_before[index];
        if (!fixture.require(polls > 0 && polls <= maximum,
            "sustained pending did not back off while retaining readiness progress")) return;
        std::cout << "PACING_BACKOFF synthetic-wait native-timers window=" << index
                  << " observed-ms=" << elapsed << " late-polls=" << polls << '\n';
      }
      phase = 3;
      for (auto& window : fixture.windows) window.api.waited = {WAIT_OBJECT_0, ERROR_SUCCESS};
      // Let the already armed delayed readiness wakes produce the actual frames.
    }
  };
  fixture.on_frame = [&](const WindowRuntimeContext&) {
    if (!fixture.require(phase == 3, "native frame arrived before test admission release")) return;
    if (fixture.windows[0].successes && fixture.windows[1].successes) {
      fixture.completed = true;
      fixture.application.quit();
    }
  };
  return fixture.run("DX12 native timers sustained pending backoff");
}
