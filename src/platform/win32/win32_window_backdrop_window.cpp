#include "win32_window_internal.hpp"

namespace cgpui {

PlatformWindowBackdropState Win32Window::apply_window_backdrop(
    WindowBackdrop backdrop, bool transparent_surface) {
  const bool layered = (chrome_state_.extended_style & WS_EX_LAYERED) != 0;
  return backdrop_.apply(hwnd_, backdrop, transparent_surface, layered);
}

PlatformWindowBackdropState Win32Window::window_backdrop_state() const {
  return backdrop_.state();
}

} // namespace cgpui
