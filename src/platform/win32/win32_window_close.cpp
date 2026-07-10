#include "win32_window_internal.hpp"

namespace cgpui {

void Win32Window::request_close() {
  if (hwnd_ != nullptr) {
    PostMessageW(hwnd_, WM_CLOSE, win32_application_close_wparam, 0);
  }
}

void Win32Window::close_requested(WindowCloseRequestSource source) {
  if (!close_controller_.begin(source)) {
    return;
  }
  state_.close_requested = true;
  callback_(close_controller_.event());
}

PlatformWindowCloseState Win32Window::close_request_state() const {
  return close_controller_.state();
}

bool Win32Window::resolve_close_request(
    PlatformWindowCloseResolution resolution) {
  if (!close_controller_.resolve(resolution)) {
    return false;
  }
  if (resolution == PlatformWindowCloseResolution::cancel) {
    state_.close_requested = false;
  }
  return true;
}

} // namespace cgpui
