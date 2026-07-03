#include "win32_window_internal.hpp"

namespace cgpui {

void Win32Window::update_size() {
  const auto dpi = static_cast<float>(GetDpiForWindow(hwnd_));
  update_size_for_dpi(dpi);
}

void Win32Window::update_size_for_dpi(float dpi) {
  RECT rect{};
  GetClientRect(hwnd_, &rect);
  const auto width = static_cast<float>(rect.right - rect.left);
  const auto height = static_cast<float>(rect.bottom - rect.top);
  state_.framebuffer_size = Size{width, height};
  state_.scale = DpiScale{dpi / 96.0F};
  callback_(WindowResized{.size = state_.framebuffer_size, .scale = state_.scale});
}

void Win32Window::dpi_changed(WPARAM wparam, LPARAM lparam) {
  if (lparam != 0) {
    const auto* rect = reinterpret_cast<const RECT*>(lparam);
    SetWindowPos(
        hwnd_,
        nullptr,
        rect->left,
        rect->top,
        rect->right - rect->left,
        rect->bottom - rect->top,
        SWP_NOZORDER | SWP_NOACTIVATE);
  }
  update_size_for_dpi(static_cast<float>(HIWORD(wparam)));
}

} // namespace cgpui
