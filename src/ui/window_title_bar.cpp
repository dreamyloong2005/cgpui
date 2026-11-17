#include "cgpui/app/window.hpp"

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

PlatformWindowFrameMetrics Window::frame_metrics() const {
  const WindowRuntimeRecord* runtime_record = record();
  if (runtime_record == nullptr || runtime_record->window == nullptr) {
    return PlatformWindowFrameMetrics{.reason = "native window is not active"};
  }
  return runtime_record->window->frame_metrics();
}

PlatformWindowActionResult Window::perform_window_action(
    PlatformWindowActionRequest request) const {
  const WindowRuntimeRecord* runtime_record = record();
  if (runtime_record == nullptr || runtime_record->window == nullptr) {
    return PlatformWindowActionResult{.reason = "native window is not active"};
  }
  return runtime_record->window->perform_window_action(request);
}

} // namespace cgpui
