#include "cgpui/platform/platform_window.hpp"

namespace cgpui {

PlatformPointerCaptureState PlatformWindow::pointer_capture_state() const {
  return {};
}

void PlatformWindow::set_pointer_capture(bool captured) {
  (void)captured;
}

} // namespace cgpui
