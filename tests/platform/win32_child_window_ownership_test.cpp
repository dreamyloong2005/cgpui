#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }
  auto parent = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Parent"},
      [](const cgpui::PlatformEvent&) {});
  if (!parent) {
    return 2;
  }
  auto child = (*app)->create_child_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Child"},
      **parent,
      [](const cgpui::PlatformEvent&) {});
  if (!child) {
    return 3;
  }
  const auto parent_surface = std::get<cgpui::Win32SurfaceHandle>(
      (*parent)->native_surface());
  const auto child_surface = std::get<cgpui::Win32SurfaceHandle>(
      (*child)->native_surface());
  const auto parent_hwnd = static_cast<HWND>(parent_surface.hwnd);
  const auto child_hwnd = static_cast<HWND>(child_surface.hwnd);
  if (parent_hwnd == nullptr || child_hwnd == nullptr) {
    return 4;
  }
  return GetWindow(child_hwnd, GW_OWNER) == parent_hwnd &&
          (GetWindowLongPtrW(child_hwnd, GWL_STYLE) & WS_CHILD) == 0
      ? 0
      : 5;
}
