#include "win32_window_internal.hpp"
#include "win32_popup_window_internal.hpp"
#include "win32_window_factory_internal.hpp"
#include "win32_window_registry_internal.hpp"

#include <utility>

namespace cgpui {

Win32Window::Win32Window(
    HINSTANCE instance,
    PlatformEventCallback callback,
    WindowState state)
    : Win32WindowDispatchLifetime(std::move(callback)),
      instance_(instance),
      state_(state),
      ole_drop_target_(std::make_unique<Win32OleDropTarget>(*this)) {
  configure_accessibility_actions();
}

Win32Window::~Win32Window() {
  invalidate_window_registration();
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
  backdrop_.detach();
  hwnd_ = nullptr;
}

NativeSurfaceHandle Win32Window::native_surface() const {
  return Win32SurfaceHandle{.hinstance = instance_,
                            .hwnd = hwnd_,
                            .composition = win32_is_popup_window(hwnd_)};
}

HWND win32_window_hwnd(const PlatformWindow& window) {
  const NativeSurfaceHandle surface = window.native_surface();
  const auto* win32_surface = std::get_if<Win32SurfaceHandle>(&surface);
  return win32_surface == nullptr ? nullptr : static_cast<HWND>(win32_surface->hwnd);
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
  // A wakeup can reclaim this very window. Keep the executing callable
  // alive independently of its owning Win32Window.
  const PlatformEventCallback callback = callback_;
  callback(WindowWakeupRequested{});
}

} // namespace cgpui
