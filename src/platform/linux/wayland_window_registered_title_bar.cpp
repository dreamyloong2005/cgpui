#include "wayland_window_internal.hpp"

#include <utility>

namespace cgpui {

void RegisteredWaylandWindow::set_window_control_regions(
    std::vector<WindowControlRegion> regions) {
  window_->set_window_control_regions(std::move(regions));
}

PlatformWindowFrameMetrics RegisteredWaylandWindow::frame_metrics() const {
  return window_->frame_metrics();
}

PlatformWindowActionResult RegisteredWaylandWindow::perform_window_action(
    PlatformWindowActionRequest request) {
  return window_->perform_window_action(request);
}

} // namespace cgpui
