#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <variant>

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }

  bool close_event = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Lifecycle State Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        close_event = close_event ||
                      std::holds_alternative<cgpui::WindowCloseRequested>(event);
      });
  if (!window) {
    return 2;
  }

  const cgpui::PlatformWindowLifecycleState initial =
      (*window)->lifecycle_state();
  if (!initial.native_window_created ||
      !initial.initial_configure_complete || initial.close_requested ||
      initial.display_state != cgpui::PlatformWindowDisplayState::normal) {
    return 3;
  }

  const auto surface =
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface());
  auto* hwnd = static_cast<HWND>(surface.hwnd);
  if (hwnd == nullptr) {
    return 4;
  }
  SendMessageW(hwnd, WM_CLOSE, 0, 0);

  const cgpui::PlatformWindowLifecycleState closing =
      (*window)->lifecycle_state();
  if (!close_event || !closing.close_requested) {
    return 5;
  }
  return 0;
}
