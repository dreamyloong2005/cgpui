#include "win32_window_internal.hpp"

#include "win32_pointer_capture_internal.hpp"

namespace cgpui {

PlatformPointerCaptureState Win32Window::pointer_capture_state() const {
  return {
      .supported = true,
      .captured = win32_pointer_captured(hwnd_),
  };
}

void Win32Window::set_pointer_capture(bool captured) {
  if (captured) {
    capture_win32_pointer(hwnd_);
    return;
  }
  suppress_pointer_capture_lost_ = true;
  (void)release_win32_pointer(hwnd_);
  suppress_pointer_capture_lost_ = false;
}

void Win32Window::pointer_capture_lost() {
  if (!suppress_pointer_capture_lost_) {
    callback_(PointerCaptureChanged{.captured = false});
  }
}

} // namespace cgpui
