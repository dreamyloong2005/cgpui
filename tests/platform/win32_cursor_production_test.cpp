#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <array>
#include <utility>
#include <variant>

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Cursor Production"},
      [](const cgpui::PlatformEvent&) {});
  if (!window) {
    return 2;
  }
  const auto surface =
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface());
  const auto hwnd = static_cast<HWND>(surface.hwnd);
  if (hwnd == nullptr) {
    return 3;
  }

  constexpr std::array mappings{
      std::pair{cgpui::CursorShape::resize_north_west_south_east, 32642U},
      std::pair{cgpui::CursorShape::resize_north_east_south_west, 32643U},
      std::pair{cgpui::CursorShape::resize_all, 32646U},
      std::pair{cgpui::CursorShape::wait, 32514U},
      std::pair{cgpui::CursorShape::progress, 32650U},
      std::pair{cgpui::CursorShape::help, 32651U},
      std::pair{cgpui::CursorShape::up_arrow, 32516U},
  };
  for (const auto& [shape, id] : mappings) {
    (*window)->set_cursor(shape);
    const auto expected = reinterpret_cast<LONG_PTR>(
        LoadCursorW(nullptr, MAKEINTRESOURCEW(id)));
    if (GetClassLongPtrW(hwnd, GCLP_HCURSOR) != expected) {
      return 4;
    }
  }

  (*window)->set_cursor(cgpui::CursorShape::resize_all);
  const auto move_cursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32646));
  SetCursor(LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)));
  if (SendMessageW(
          hwnd,
          WM_SETCURSOR,
          reinterpret_cast<WPARAM>(hwnd),
          MAKELPARAM(HTCLIENT, WM_MOUSEMOVE)) != TRUE ||
      GetCursor() != move_cursor) {
    return 5;
  }

  (*window)->set_cursor(cgpui::CursorShape::wait);
  const auto wait_cursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32514));
  SetClassLongPtrW(
      hwnd,
      GCLP_HCURSOR,
      reinterpret_cast<LONG_PTR>(
          LoadCursorW(nullptr, MAKEINTRESOURCEW(32512))));
  SendMessageW(hwnd, WM_SETTINGCHANGE, SPI_SETCURSORS, 0);
  if (GetClassLongPtrW(hwnd, GCLP_HCURSOR) !=
      reinterpret_cast<LONG_PTR>(wait_cursor)) {
    return 6;
  }
  return 0;
}
