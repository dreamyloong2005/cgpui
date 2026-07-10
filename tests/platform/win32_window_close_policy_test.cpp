#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <variant>

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }

  cgpui::PlatformWindow* observed_window = nullptr;
  HWND hwnd = nullptr;
  int close_event_count = 0;
  bool pending_state_matched = true;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Close Policy Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        const auto* close = std::get_if<cgpui::WindowCloseRequested>(&event);
        if (close == nullptr || observed_window == nullptr) {
          return;
        }
        close_event_count += 1;
        const auto state = observed_window->close_request_state();
        pending_state_matched = pending_state_matched && state.pending &&
            !state.accepted && state.source == close->source &&
            state.sequence == close->sequence;
        if (close_event_count == 1) {
          SendMessageW(hwnd, WM_CLOSE, 0, 0);
          (void)observed_window->resolve_close_request(
              cgpui::PlatformWindowCloseResolution::cancel);
        } else {
          (void)observed_window->resolve_close_request(
              cgpui::PlatformWindowCloseResolution::accept);
          (*app)->quit();
        }
      });
  if (!window) {
    return 2;
  }
  observed_window = window->get();
  const auto surface =
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface());
  hwnd = static_cast<HWND>(surface.hwnd);
  if (hwnd == nullptr) {
    return 3;
  }

  SendMessageW(hwnd, WM_CLOSE, 0, 0);
  const auto cancelled = (*window)->close_request_state();
  if (close_event_count != 1 || cancelled.pending || cancelled.accepted ||
      cancelled.source != cgpui::WindowCloseRequestSource::window_manager ||
      cancelled.sequence != 1 || cancelled.cancelled_count != 1 ||
      cancelled.coalesced_count != 1 || (*window)->state().close_requested) {
    return 4;
  }

  (*window)->request_close();
  const int run_result = (*app)->run();
  const auto accepted = (*window)->close_request_state();
  if (run_result != 0 || close_event_count != 2 || !pending_state_matched) {
    return 5;
  }
  if (accepted.pending || !accepted.accepted ||
      accepted.source != cgpui::WindowCloseRequestSource::application ||
      accepted.sequence != 2 || accepted.accepted_count != 1 ||
      accepted.cancelled_count != 1 || accepted.coalesced_count != 1 ||
      !(*window)->state().close_requested) {
    return 6;
  }
  return 0;
}
