#include "dx12_runtime_pacing_test_support.hpp"
#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !pacing_begin_debug_enable()) return dx12_fail(1, "pacing close debug/watchdog failed");
  auto application = create_platform_application();
  if (!application) return dx12_fail(2, application.error().message.c_str());
  Dx12RuntimePacingFixture fixture{.application = **application};
  unsigned phase = 0, closed_polls = 0, root_pending_before_close = 0, root_successes_before_close = 0;
  auto child_closed_at = Dx12RuntimePacingFixture::Clock::now();
  fixture.on_tick = [&](const WindowRuntimeContext& context) {
    if (fixture.factories != 2) return;
    auto& root = fixture.windows[0];
    auto& child = fixture.windows[1];
    if (phase == 0) {
      if (root.pending < 2 || child.pending < 2) return;
      if (!fixture.require(fixture.pending_without_success(), "pending fabricated a successful frame")) return;
      root.api.waited = {WAIT_OBJECT_0, ERROR_SUCCESS}; // No explicit redraw; preserve runtime readiness scheduling.
      phase = 1;
    } else if (phase == 1) {
      if (!root.successes) return;
      if (!fixture.require(!child.successes && !child.paints && child.pending,
          "child was not still pending when native close was posted")) return;
      if (!fixture.require(PostMessageW(child.hwnd, WM_CLOSE, 0, 0) != FALSE,
          "native child WM_CLOSE post failed")) return;
      phase = 2;
    } else if (phase == 2) {
      const auto* record = context.runtime.window_runtime_record(fixture.ids[1]);
      if (record && record->active) return;
      if (!fixture.require(child.closes == 1 && child.api.forwarded.closes == 1 &&
          child.api.forwarded.close_ok && !IsWindow(child.hwnd), "native child close did not retire resources")) return;
      closed_polls = child.api.polls;
      child_closed_at = Dx12RuntimePacingFixture::Clock::now();
      phase = 3;
    } else if (phase == 3) {
      if (!fixture.require(child.api.polls == closed_polls && !child.successes && !child.paints,
          "a stale readiness timer polled or painted the closed child")) return;
      if (Dx12RuntimePacingFixture::Clock::now() - child_closed_at < std::chrono::milliseconds(120)) return;
      root.api.waited = {WAIT_TIMEOUT, ERROR_SUCCESS};
      root_pending_before_close = root.pending;
      root_successes_before_close = root.successes;
      phase = 4;
      context.request_render(); // One normal user invalidation to enter pending, never a begin retry.
    } else if (phase == 4) {
      if (root.pending <= root_pending_before_close) return;
      if (!fixture.require(root.successes == root_successes_before_close && child.api.polls == closed_polls,
          "pending root fabricated success or resurrected child")) return;
      if (!fixture.require(PostMessageW(root.hwnd, WM_CLOSE, 0, 0) != FALSE,
          "native pending-root WM_CLOSE post failed")) return;
      phase = 5;
      fixture.completed = true; // run_app must now terminate through real WM_CLOSE, not test quit().
    }
  };
  const int result = fixture.run("DX12 native runtime pending close");
  if (result) return result;
  return fixture.require(phase == 5 && fixture.windows[0].closes == 1 && fixture.windows[1].closes == 1 &&
      fixture.windows[0].successes == root_successes_before_close && fixture.windows[1].api.polls == closed_polls,
      "native close delivery/cancellation did not finish without additional success") ? 0 : 3;
}
