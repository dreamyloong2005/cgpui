#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }
  bool single_left = false;
  bool double_left = false;
  bool back = false;
  bool forward = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Pointer Button Test"},
      [&](const cgpui::PlatformEvent& event) {
        const auto* button = std::get_if<cgpui::PointerButton>(&event);
        if (button == nullptr || !button->pressed) {
          return;
        }
        single_left = single_left ||
            (button->button == cgpui::MouseButton::left &&
             button->click_count == 1);
        double_left = double_left ||
            (button->button == cgpui::MouseButton::left &&
             button->click_count == 2);
        back = back || button->button == cgpui::MouseButton::back;
        forward = forward || button->button == cgpui::MouseButton::forward;
      });
  if (!window) {
    return 2;
  }
  const auto surface =
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface());
  const auto hwnd = static_cast<HWND>(surface.hwnd);
  if (hwnd == nullptr ||
      (GetClassLongPtrW(hwnd, GCL_STYLE) & CS_DBLCLKS) == 0) {
    return 3;
  }
  constexpr LPARAM position = MAKELPARAM(24, 16);
  SendMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, position);
  SendMessageW(hwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, position);
  SendMessageW(hwnd, WM_XBUTTONDOWN, MAKEWPARAM(0, XBUTTON1), position);
  SendMessageW(hwnd, WM_XBUTTONDOWN, MAKEWPARAM(0, XBUTTON2), position);
  return single_left && double_left && back && forward ? 0 : 4;
}
