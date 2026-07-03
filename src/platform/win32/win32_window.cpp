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
    revoke_drop_target();
    SetWindowLongPtrW(hwnd_, GWLP_USERDATA, 0);
    DestroyWindow(hwnd_);
    hwnd_ = nullptr;
  }
}

void Win32Window::attach(HWND hwnd) {
  hwnd_ = hwnd;
  register_drop_target(hwnd);
}

void Win32Window::detach() {
  revoke_drop_target();
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

void Win32Window::request_close() {
  if (hwnd_ != nullptr) {
    PostMessageW(hwnd_, WM_CLOSE, 0, 0);
  }
}

void Win32Window::set_title(std::string_view title) {
  const auto wide_title = widen(title);
  SetWindowTextW(hwnd_, wide_title.c_str());
}

void Win32Window::set_cursor(CursorShape cursor_shape) {
  HCURSOR cursor = LoadCursorW(nullptr, cursor_id_for(cursor_shape));
  if (cursor == nullptr) {
    cursor = LoadCursorW(nullptr, cursor_id_for(CursorShape::default_arrow));
  }
  current_cursor_ = cursor;
  if (hwnd_ != nullptr) {
    SetClassLongPtrW(hwnd_, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(cursor));
    SetCursor(cursor);
  }
}

void Win32Window::set_ime_text_input_placement(
    std::optional<ImeTextInputPlacement> placement) {
  state_.ime_text_input_placement = placement;
  apply_ime_text_input_placement();
}

void Win32Window::update_accessibility_tree(
    PlatformAccessibilityTreeUpdate update) {
  uia_accessibility_.update(std::move(update));
}

void Win32Window::close_requested() {
  state_.close_requested = true;
  callback_(WindowCloseRequested{});
}

void Win32Window::redraw_requested() {
  callback_(WindowRedrawRequested{});
}

void Win32Window::wakeup_requested() {
  callback_(WindowWakeupRequested{});
}

} // namespace cgpui
