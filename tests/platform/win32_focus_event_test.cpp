#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <variant>

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 2;
  }

  bool focused = false;
  bool blurred = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Focus Event Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* focus = std::get_if<cgpui::WindowFocused>(&event);
            focus != nullptr) {
          focused = focused || focus->focused;
          blurred = blurred || !focus->focused;
        }
      });
  if (!window) {
    return 3;
  }

  const auto surface =
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface());
  auto* hwnd = static_cast<HWND>(surface.hwnd);
  if (hwnd == nullptr) {
    return 4;
  }

  SendMessageW(hwnd, WM_SETFOCUS, 0, 0);
  SendMessageW(hwnd, WM_KILLFOCUS, 0, 0);

  if (!focused) {
    return 5;
  }
  if (!blurred) {
    return 6;
  }

  return 0;
}
