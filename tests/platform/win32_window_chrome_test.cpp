#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }
  const cgpui::WindowChromeOptions frameless{
      .titlebar_visible = false,
      .decorations = false,
      .resizable = false,
      .transparent_background = true};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Window Chrome Test",
          .size = {420.0F, 280.0F},
          .chrome = frameless},
      [](const cgpui::PlatformEvent&) {});
  if (!window) {
    return 2;
  }
  const cgpui::NativeSurfaceHandle native_surface = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::Win32SurfaceHandle>(&native_surface);
  if (surface == nullptr || surface->hwnd == nullptr) {
    return 3;
  }
  const auto hwnd = static_cast<HWND>(surface->hwnd);
  const LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
  const LONG_PTR extended_style = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  if ((style & (WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX)) != 0 ||
      (extended_style & WS_EX_LAYERED) == 0) {
    return 4;
  }
  const auto applied = (*window)->apply_window_chrome({});
  if (!applied.supported || !applied.decoration_control_supported ||
      !applied.transparency_supported ||
      (GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_OVERLAPPEDWINDOW) == 0 ||
      (GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED) != 0) {
    return 5;
  }
  return 0;
}
