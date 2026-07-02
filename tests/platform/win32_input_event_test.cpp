#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <ole2.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <variant>

namespace {

constexpr cgpui::Point expected_position{31.0F, 47.0F};
constexpr std::uint32_t expected_key = 'A';

struct Win32DragDropTestPayload {
  float x = 0.0F;
  float y = 0.0F;
  const wchar_t* text = nullptr;
  const wchar_t* const* files = nullptr;
  std::size_t file_count = 0;
  std::uint32_t drop_effect = 0;
};

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
  bool drag_entered = false;
  bool drag_updated = false;
  bool drag_dropped = false;
  bool drag_exited = false;
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
        if (const auto* drag = std::get_if<cgpui::DragEntered>(&event);
            drag != nullptr && point_equals(drag->position, expected_position)) {
          drag_entered =
              drag->payload.kind == cgpui::DragDropPayloadKind::text &&
              drag->payload.text == "Dragged text" &&
              drag->payload.files.empty() &&
              drag->action == cgpui::DragDropAction::copy;
        }
        if (const auto* drag = std::get_if<cgpui::DragUpdated>(&event);
            drag != nullptr && point_equals(drag->position, expected_position)) {
          drag_updated =
              drag->payload.kind == cgpui::DragDropPayloadKind::text &&
              drag->payload.text == "Dragged text" &&
              drag->payload.files.empty() &&
              drag->action == cgpui::DragDropAction::move;
        }
        if (const auto* drag = std::get_if<cgpui::DragDropped>(&event);
            drag != nullptr && point_equals(drag->position, expected_position)) {
          drag_dropped =
              drag->payload.kind == cgpui::DragDropPayloadKind::files &&
              drag->payload.text.empty() && drag->payload.files.size() == 2 &&
              drag->payload.files[0] == "C:\\Temp\\first.txt" &&
              drag->payload.files[1] == "C:\\Temp\\second.cpp" &&
              drag->action == cgpui::DragDropAction::move;
        }
        if (const auto* drag = std::get_if<cgpui::DragExited>(&event);
            drag != nullptr && point_equals(drag->position, expected_position)) {
          drag_exited =
              drag->payload.kind == cgpui::DragDropPayloadKind::none &&
              drag->payload.text.empty() && drag->payload.files.empty() &&
              drag->action == cgpui::DragDropAction::none;
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
  const UINT drag_enter_message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragEnter");
  const UINT drag_update_message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragUpdate");
  const UINT drag_drop_message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragDrop");
  const UINT drag_exit_message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragExit");
  const Win32DragDropTestPayload text_copy_payload{
      .x = expected_position.x,
      .y = expected_position.y,
      .text = L"Dragged text",
      .drop_effect = DROPEFFECT_COPY,
  };
  const Win32DragDropTestPayload text_move_payload{
      .x = expected_position.x,
      .y = expected_position.y,
      .text = L"Dragged text",
      .drop_effect = DROPEFFECT_MOVE,
  };
  const wchar_t* file_paths[] = {
      L"C:\\Temp\\first.txt",
      L"C:\\Temp\\second.cpp",
  };
  const Win32DragDropTestPayload file_payload{
      .x = expected_position.x,
      .y = expected_position.y,
      .files = file_paths,
      .file_count = 2,
      .drop_effect = DROPEFFECT_MOVE,
  };
  const Win32DragDropTestPayload empty_payload{
      .x = expected_position.x,
      .y = expected_position.y,
  };
  SendMessageW(
      hwnd,
      drag_enter_message,
      0,
      reinterpret_cast<LPARAM>(&text_copy_payload));
  SendMessageW(
      hwnd,
      drag_update_message,
      0,
      reinterpret_cast<LPARAM>(&text_move_payload));
  SendMessageW(
      hwnd,
      drag_drop_message,
      0,
      reinterpret_cast<LPARAM>(&file_payload));
  SendMessageW(
      hwnd,
      drag_exit_message,
      0,
      reinterpret_cast<LPARAM>(&empty_payload));

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
  if (!drag_entered) {
    return 12;
  }
  if (!drag_updated) {
    return 13;
  }
  if (!drag_dropped) {
    return 14;
  }
  if (!drag_exited) {
    return 15;
  }

  return 0;
}
