#include "win32_appearance_monitor_internal.hpp"

#include <utility>

namespace cgpui {

Win32AppearanceMonitor::Win32AppearanceMonitor(HINSTANCE instance)
    : instance_(instance), last_appearance_(win32_read_system_appearance()) {}

Win32AppearanceMonitor::~Win32AppearanceMonitor() {
  if (hwnd_ != nullptr) {
    SetWindowLongPtrW(hwnd_, GWLP_USERDATA, 0);
    DestroyWindow(hwnd_);
    hwnd_ = nullptr;
  }
}

bool Win32AppearanceMonitor::ensure_started() {
  if (hwnd_ != nullptr) return true;
  WNDCLASSEXW window_class{};
  window_class.cbSize = sizeof(WNDCLASSEXW);
  window_class.lpfnWndProc = &Win32AppearanceMonitor::window_proc;
  window_class.hInstance = instance_;
  window_class.lpszClassName = win32_appearance_monitor_class;
  if (RegisterClassExW(&window_class) == 0 &&
      GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
    return false;
  }
  // Hidden, never shown, and excluded from the taskbar by WS_EX_TOOLWINDOW.
  hwnd_ = CreateWindowExW(
      WS_EX_TOOLWINDOW, win32_appearance_monitor_class, L"", WS_POPUP, 0, 0,
      0, 0, nullptr, nullptr, instance_, this);
  return hwnd_ != nullptr;
}

void Win32AppearanceMonitor::set_callback(SystemAppearanceCallback callback) {
  callback_ = std::move(callback);
}

void Win32AppearanceMonitor::handle(SystemAppearanceChangeSource source) {
  SystemAppearance current = win32_read_system_appearance();
  const bool changed = !(current == last_appearance_);
  last_appearance_ = current;
  win32_apply_title_bar_appearance_to_thread_windows(
      current.mode == SystemAppearanceMode::dark);
  if (!callback_) return;
  const SystemAppearanceCallback callback = callback_;
  callback(SystemAppearanceChange{
      .source = source, .appearance = std::move(current), .changed = changed});
}

LRESULT CALLBACK Win32AppearanceMonitor::window_proc(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == WM_NCCREATE) {
    const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lparam);
    SetWindowLongPtrW(
        hwnd, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(create->lpCreateParams));
    return DefWindowProcW(hwnd, message, wparam, lparam);
  }
  auto* monitor = reinterpret_cast<Win32AppearanceMonitor*>(
      GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (monitor != nullptr) {
    if (const auto source =
            win32_appearance_change_source(message, wparam, lparam)) {
      monitor->handle(*source);
      return 0;
    }
  }
  return DefWindowProcW(hwnd, message, wparam, lparam);
}

} // namespace cgpui
