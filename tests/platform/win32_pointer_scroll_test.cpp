#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <cmath>
#include <variant>
#include <vector>

namespace {

bool point_equals(cgpui::Point lhs, cgpui::Point rhs) {
  return std::fabs(lhs.x - rhs.x) < 0.01F &&
      std::fabs(lhs.y - rhs.y) < 0.01F;
}

LPARAM screen_point_lparam(HWND hwnd, cgpui::Point point) {
  POINT screen_point{
      .x = static_cast<LONG>(point.x),
      .y = static_cast<LONG>(point.y),
  };
  ClientToScreen(hwnd, &screen_point);
  return MAKELPARAM(
      static_cast<SHORT>(screen_point.x),
      static_cast<SHORT>(screen_point.y));
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }
  std::vector<cgpui::PointerScrolled> events;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Pointer Scroll Test"},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* scroll = std::get_if<cgpui::PointerScrolled>(&event)) {
          events.push_back(*scroll);
        }
      });
  if (!window) {
    return 2;
  }
  const auto surface =
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface());
  const auto hwnd = static_cast<HWND>(surface.hwnd);
  if (hwnd == nullptr) {
    return 3;
  }
  constexpr cgpui::Point position{31.0F, 47.0F};
  const LPARAM screen_position = screen_point_lparam(hwnd, position);
  SendMessageW(
      hwnd, WM_MOUSEWHEEL, MAKEWPARAM(0, WHEEL_DELTA), screen_position);
  SendMessageW(
      hwnd, WM_MOUSEHWHEEL, MAKEWPARAM(0, WHEEL_DELTA), screen_position);
  SendMessageW(
      hwnd,
      WM_MOUSEWHEEL,
      MAKEWPARAM(0, WHEEL_DELTA / 4),
      screen_position);
  if (events.size() != 3) {
    return 4;
  }
  if (!point_equals(events[0].delta, {0.0F, 1.0F}) ||
      events[0].precise || !point_equals(events[0].position, position)) {
    return 5;
  }
  if (!point_equals(events[1].delta, {1.0F, 0.0F}) ||
      events[1].precise || !point_equals(events[1].position, position)) {
    return 6;
  }
  if (!point_equals(events[2].delta, {0.0F, 0.25F}) ||
      !events[2].precise || !point_equals(events[2].position, position)) {
    return 7;
  }
  return 0;
}
