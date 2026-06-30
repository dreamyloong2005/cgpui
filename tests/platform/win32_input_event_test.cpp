#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <cmath>
#include <cstdint>
#include <variant>

namespace {

constexpr cgpui::Point expected_position{31.0F, 47.0F};
constexpr std::uint32_t expected_key = 'A';

bool point_equals(cgpui::Point lhs, cgpui::Point rhs) {
  return std::fabs(lhs.x - rhs.x) < 0.01F && std::fabs(lhs.y - rhs.y) < 0.01F;
}

LPARAM client_point_lparam(cgpui::Point point) {
  return MAKELPARAM(
      static_cast<SHORT>(point.x),
      static_cast<SHORT>(point.y));
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
    return 2;
  }

  bool moved = false;
  bool pressed = false;
  bool released = false;
  bool scrolled = false;
  bool key_pressed = false;
  bool key_released = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Input Event Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* move = std::get_if<cgpui::PointerMoved>(&event);
            move != nullptr && point_equals(move->position, expected_position)) {
          moved = true;
        }
        if (const auto* button = std::get_if<cgpui::PointerButton>(&event);
            button != nullptr && button->button == cgpui::MouseButton::left &&
            point_equals(button->position, expected_position)) {
          pressed = pressed || button->pressed;
          released = released || !button->pressed;
        }
        if (const auto* scroll = std::get_if<cgpui::PointerScrolled>(&event);
            scroll != nullptr && point_equals(scroll->position, expected_position) &&
            point_equals(scroll->delta, cgpui::Point{0.0F, 1.0F})) {
          scrolled = true;
        }
        if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
            key != nullptr && key->key_code == expected_key) {
          key_pressed = key_pressed || key->action == cgpui::KeyAction::pressed;
          key_released = key_released || key->action == cgpui::KeyAction::released;
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

  (*window)->set_cursor(cgpui::CursorShape::text);
  const auto expected_text_cursor =
      reinterpret_cast<LONG_PTR>(LoadCursorW(nullptr, MAKEINTRESOURCEW(32513)));
  const auto applied_text_cursor = GetClassLongPtrW(hwnd, GCLP_HCURSOR);
  if (applied_text_cursor != expected_text_cursor) {
    return 11;
  }

  SendMessageW(
      hwnd,
      WM_MOUSEMOVE,
      0,
      client_point_lparam(expected_position));
  SendMessageW(
      hwnd,
      WM_LBUTTONDOWN,
      MK_LBUTTON,
      client_point_lparam(expected_position));
  SendMessageW(
      hwnd,
      WM_LBUTTONUP,
      0,
      client_point_lparam(expected_position));
  SendMessageW(
      hwnd,
      WM_MOUSEWHEEL,
      MAKEWPARAM(0, WHEEL_DELTA),
      screen_point_lparam(hwnd, expected_position));
  SendMessageW(hwnd, WM_KEYDOWN, expected_key, 0);
  SendMessageW(hwnd, WM_KEYUP, expected_key, 0);

  if (!moved) {
    return 5;
  }
  if (!pressed || !released) {
    return 6;
  }
  if (!scrolled) {
    return 7;
  }
  if (!key_pressed || !key_released) {
    return 8;
  }

  return 0;
}
