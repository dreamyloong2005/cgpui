#include "dx12_runtime_pacing_test_support.hpp"
#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !pacing_begin_debug_enable()) return dx12_fail(1, "pacing runtime debug/watchdog failed");
  auto application = create_platform_application();
  if (!application) return dx12_fail(2, application.error().message.c_str());
  Dx12RuntimePacingFixture fixture{.application = **application};
  unsigned phase = 0;
  std::array<unsigned, 2> polls_after_input{};
  auto actions_at = Dx12RuntimePacingFixture::Clock::now();
  fixture.on_tick = [&](const WindowRuntimeContext&) {
    if (fixture.factories != 2) return;
    auto& windows = fixture.windows;
    if (phase == 0) {
      if (windows[0].pending < 2 || windows[1].pending < 2) return;
      if (!fixture.require(fixture.pending_without_success(), "pending produced paint/capture/success/error")) return;
      for (auto& window : windows) {
        const auto old_width = window.width, old_height = window.height, old_resizes = window.resizes;
        RECT outer{}, client{};
        if (!fixture.require(GetWindowRect(window.hwnd, &outer) &&
            SetWindowPos(window.hwnd, nullptr, 0, 0,
                outer.right - outer.left + 64, outer.bottom - outer.top + 48,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE) && GetClientRect(window.hwnd, &client),
            "actual SetWindowPos resize failed")) return;
        if (!fixture.require(window.width > old_width && window.height > old_height &&
            window.resizes > old_resizes && window.width == static_cast<unsigned>(client.right) &&
            window.height == static_cast<unsigned>(client.bottom) && window.api.forwarded.gets == 1,
            "native WM_SIZE did not resize the real renderer on its existing chain")) return;
        if (!fixture.require(PostMessageW(window.hwnd, WM_KEYDOWN, VK_F8, 1) != FALSE,
            "native queued keyboard input failed")) return;
      }
      polls_after_input = {windows[0].api.polls, windows[1].api.polls};
      actions_at = Dx12RuntimePacingFixture::Clock::now();
      phase = 1;
    } else if (phase == 1) {
      if (Dx12RuntimePacingFixture::Clock::now() - actions_at < std::chrono::milliseconds(80)) return;
      if (!fixture.require(fixture.pending_without_success() && windows[0].keys == 1 && windows[1].keys == 1 &&
          windows[0].key_routes >= 1 && windows[1].key_routes >= 1 &&
          windows[0].api.polls >= polls_after_input[0] + 2 &&
          windows[1].api.polls >= polls_after_input[1] + 2,
          "pending starved input, independent user timer or either window readiness redraw")) return;
      phase = 2;
      for (auto& window : windows) window.api.waited = {WAIT_OBJECT_0, ERROR_SUCCESS};
      // No request_render here: the runtime's previously armed readiness path must wake both windows.
    }
  };
  fixture.on_frame = [&](const WindowRuntimeContext&) {
    if (!fixture.require(phase == 2, "success published before the synthetic grant was released")) return;
    if (fixture.windows[0].successes && fixture.windows[1].successes) {
      fixture.completed = true;
      fixture.application.quit();
    }
  };
  return fixture.run("DX12 native runtime pending fairness");
}
