#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <cmath>
#include <variant>
#include <vector>

namespace {

bool nearly_equal(float lhs, float rhs) {
  return std::fabs(lhs - rhs) < 0.01F;
}

bool point_equal(cgpui::Point lhs, cgpui::Point rhs) {
  return nearly_equal(lhs.x, rhs.x) && nearly_equal(lhs.y, rhs.y);
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }
  std::vector<cgpui::PointerMoved> moves;
  std::vector<cgpui::PointerButton> buttons;
  std::vector<cgpui::PointerScrolled> scrolls;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Input DPI"},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* moved = std::get_if<cgpui::PointerMoved>(&event)) {
          moves.push_back(*moved);
        } else if (const auto* button =
                       std::get_if<cgpui::PointerButton>(&event)) {
          buttons.push_back(*button);
        } else if (const auto* scroll =
                       std::get_if<cgpui::PointerScrolled>(&event)) {
          scrolls.push_back(*scroll);
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

  const unsigned int dpi = nearly_equal((*window)->state().scale.value, 2.0F)
      ? 96U
      : 192U;
  const float scale = static_cast<float>(dpi) / 96.0F;
  RECT suggested{};
  GetWindowRect(hwnd, &suggested);
  SendMessageW(
      hwnd,
      WM_DPICHANGED,
      MAKEWPARAM(dpi, dpi),
      reinterpret_cast<LPARAM>(&suggested));
  if (!nearly_equal((*window)->state().scale.value, scale)) {
    return 4;
  }

  constexpr cgpui::Point logical_position{40.0F, 30.0F};
  const POINT client_point{
      .x = static_cast<LONG>(logical_position.x * scale),
      .y = static_cast<LONG>(logical_position.y * scale),
  };
  const LPARAM client_lparam = MAKELPARAM(client_point.x, client_point.y);
  POINT screen_point = client_point;
  ClientToScreen(hwnd, &screen_point);
  const LPARAM screen_lparam = MAKELPARAM(screen_point.x, screen_point.y);

  SendMessageW(hwnd, WM_MOUSEMOVE, 0, client_lparam);
  SendMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, client_lparam);
  SendMessageW(hwnd, WM_LBUTTONUP, 0, client_lparam);
  SendMessageW(
      hwnd, WM_MOUSEWHEEL, MAKEWPARAM(0, WHEEL_DELTA), screen_lparam);

  if (moves.size() != 1 || !point_equal(moves[0].position, logical_position)) {
    return 5;
  }
  if (buttons.size() != 2 ||
      !point_equal(buttons[0].position, logical_position) ||
      !point_equal(buttons[1].position, logical_position)) {
    return 6;
  }
  if (scrolls.size() != 1 ||
      !point_equal(scrolls[0].position, logical_position) ||
      !point_equal(scrolls[0].delta, {0.0F, 1.0F})) {
    return 7;
  }
  return 0;
}
