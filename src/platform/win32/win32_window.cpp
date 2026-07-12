#include "win32_window_internal.hpp"

#include <utility>

namespace cgpui {

Win32Window::Win32Window(
    HINSTANCE instance,
    PlatformEventCallback callback,
    WindowState state)
    : instance_(instance),
      callback_(std::move(callback)),
      state_(state),
      ole_drop_target_(std::make_unique<Win32OleDropTarget>(*this)) {}

Win32Window::~Win32Window() {
  if (hwnd_ != nullptr) {
    set_pointer_capture(false);
    revoke_drop_target();
    SetWindowLongPtrW(hwnd_, GWLP_USERDATA, 0);
    DestroyWindow(hwnd_);
    hwnd_ = nullptr;
  }
}

void Win32Window::attach(HWND hwnd) {
  hwnd_ = hwnd;
  uia_accessibility_.attach(hwnd);
  refresh_cursor(true);
  register_drop_target(hwnd);
}

void Win32Window::detach() {
  set_pointer_capture(false);
  revoke_drop_target();
  uia_accessibility_.detach();
  hwnd_ = nullptr;
}

NativeSurfaceHandle Win32Window::native_surface() const {
  return Win32SurfaceHandle{.hinstance = instance_, .hwnd = hwnd_};
}

WindowState Win32Window::state() const {
  return state_;
}

void Win32Window::request_redraw() {
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void Win32Window::set_title(std::string_view title) {
  const auto wide_title = widen(title);
  SetWindowTextW(hwnd_, wide_title.c_str());
}

void Win32Window::set_ime_text_input_placement(
    std::optional<ImeTextInputPlacement> placement) {
  state_.ime_text_input_placement = placement;
  apply_ime_text_input_placement();
}

void Win32Window::redraw_requested() {
  callback_(WindowRedrawRequested{});
}

void Win32Window::wakeup_requested() {
  callback_(WindowWakeupRequested{});
}

} // namespace cgpui
