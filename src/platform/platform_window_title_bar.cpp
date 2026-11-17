#include "cgpui/platform/platform_window.hpp"

#include <utility>

namespace cgpui {
namespace {

PlatformWindowActionResult display_result(bool performed) {
  return PlatformWindowActionResult{
      .supported = performed,
      .performed = performed,
      .reason = performed ? std::string{}
                          : "the backend rejected the display state request",
  };
}

} // namespace

void PlatformWindow::set_window_control_regions(
    std::vector<WindowControlRegion> regions) {
  (void)std::move(regions);
}

PlatformWindowFrameMetrics PlatformWindow::frame_metrics() const {
  return PlatformWindowFrameMetrics{
      .supported = false,
      .backend = "unsupported",
      .reason = "client title bar unsupported on this backend",
  };
}

// Minimize, maximize and close reuse the display-state and close requests
// every backend already implements. Interactive move, resize and the window
// menu need native support and are unsupported here.
PlatformWindowActionResult PlatformWindow::perform_window_action(
    PlatformWindowActionRequest request) {
  switch (request.action) {
    case PlatformWindowAction::minimize:
      return display_result(
          request_display_state(PlatformWindowDisplayState::minimized));
    case PlatformWindowAction::toggle_maximize:
      return display_result(request_display_state(
          lifecycle_state().display_state ==
                  PlatformWindowDisplayState::maximized
              ? PlatformWindowDisplayState::normal
              : PlatformWindowDisplayState::maximized));
    case PlatformWindowAction::close:
      request_close();
      return PlatformWindowActionResult{.supported = true, .performed = true};
    case PlatformWindowAction::show_window_menu:
    case PlatformWindowAction::begin_move:
    case PlatformWindowAction::begin_resize:
      break;
  }
  return PlatformWindowActionResult{
      .reason = "interactive window actions unsupported on this backend"};
}

} // namespace cgpui
