#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <variant>

namespace {

bool rect_equals(const RECT& lhs, const RECT& rhs) {
  return lhs.left == rhs.left && lhs.top == rhs.top &&
      lhs.right == rhs.right && lhs.bottom == rhs.bottom;
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }

  cgpui::PlatformWindow* observed_window = nullptr;
  bool minimized_before_resize = false;
  bool maximized_before_resize = false;
  bool fullscreen_before_resize = false;
  bool restored_before_resize = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Display State Test",
          .size = cgpui::Size{420.0F, 280.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (observed_window == nullptr ||
            !std::holds_alternative<cgpui::WindowResized>(event)) {
          return;
        }
        switch (observed_window->lifecycle_state().display_state) {
        case cgpui::PlatformWindowDisplayState::minimized:
          minimized_before_resize = true;
          break;
        case cgpui::PlatformWindowDisplayState::maximized:
          maximized_before_resize = true;
          break;
        case cgpui::PlatformWindowDisplayState::fullscreen:
          fullscreen_before_resize = true;
          break;
        case cgpui::PlatformWindowDisplayState::normal:
          restored_before_resize = true;
          break;
        }
      });
  if (!window) {
    return 2;
  }
  observed_window = window->get();

  const auto surface =
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface());
  auto* hwnd = static_cast<HWND>(surface.hwnd);
  if (hwnd == nullptr) {
    return 3;
  }
  const auto initial_style = GetWindowLongPtrW(hwnd, GWL_STYLE);
  const auto initial_extended_style = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  RECT initial_rect{};
  if (GetWindowRect(hwnd, &initial_rect) == FALSE) {
    return 4;
  }

  if (!(*window)->request_display_state(
          cgpui::PlatformWindowDisplayState::minimized) ||
      IsIconic(hwnd) == FALSE ||
      (*window)->lifecycle_state().display_state !=
          cgpui::PlatformWindowDisplayState::minimized) {
    return 5;
  }
  if (!(*window)->request_display_state(
          cgpui::PlatformWindowDisplayState::maximized) ||
      IsZoomed(hwnd) == FALSE ||
      (*window)->lifecycle_state().display_state !=
          cgpui::PlatformWindowDisplayState::maximized) {
    return 6;
  }
  if (!(*window)->request_display_state(
          cgpui::PlatformWindowDisplayState::normal) ||
      IsIconic(hwnd) != FALSE || IsZoomed(hwnd) != FALSE ||
      (*window)->lifecycle_state().display_state !=
          cgpui::PlatformWindowDisplayState::normal) {
    return 7;
  }

  if (!(*window)->request_display_state(
          cgpui::PlatformWindowDisplayState::fullscreen)) {
    return 8;
  }
  MONITORINFO monitor{.cbSize = sizeof(MONITORINFO)};
  RECT fullscreen_rect{};
  if (GetMonitorInfoW(
          MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &monitor) == FALSE ||
      GetWindowRect(hwnd, &fullscreen_rect) == FALSE ||
      !rect_equals(fullscreen_rect, monitor.rcMonitor) ||
      (GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_OVERLAPPEDWINDOW) != 0 ||
      (*window)->lifecycle_state().display_state !=
          cgpui::PlatformWindowDisplayState::fullscreen) {
    return 9;
  }
  if (!(*window)->request_display_state(
          cgpui::PlatformWindowDisplayState::normal)) {
    return 10;
  }
  RECT restored_rect{};
  if (GetWindowRect(hwnd, &restored_rect) == FALSE ||
      !rect_equals(restored_rect, initial_rect) ||
      GetWindowLongPtrW(hwnd, GWL_STYLE) != initial_style ||
      GetWindowLongPtrW(hwnd, GWL_EXSTYLE) != initial_extended_style ||
      (*window)->lifecycle_state().display_state !=
          cgpui::PlatformWindowDisplayState::normal) {
    return 11;
  }
  if (!minimized_before_resize || !maximized_before_resize ||
      !fullscreen_before_resize || !restored_before_resize) {
    return 12;
  }
  return 0;
}
