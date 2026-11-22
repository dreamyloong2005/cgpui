#include "win32_window_internal.hpp"

namespace cgpui {

PlatformWindowBackdropState Win32Window::apply_window_backdrop(
    WindowBackdrop backdrop, bool transparent_surface) {
  const bool client_title_bar =
      chrome_state_.platform.applied.title_bar == WindowTitleBarMode::client;
  return backdrop_.apply(hwnd_, backdrop, transparent_surface, client_title_bar);
}

PlatformWindowBackdropState Win32Window::window_backdrop_state() const {
  return backdrop_.state();
}

} // namespace cgpui
