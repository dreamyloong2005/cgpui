#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <variant>

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }

  cgpui::PlatformWindow* observed_window = nullptr;
  int capture_lost_count = 0;
  bool loss_saw_released_state = false;
  cgpui::Point last_pointer_position{};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Pointer Capture"},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* changed =
                std::get_if<cgpui::PointerCaptureChanged>(&event);
            changed != nullptr && !changed->captured) {
          capture_lost_count += 1;
          loss_saw_released_state =
              observed_window != nullptr &&
              !observed_window->pointer_capture_state().captured;
        } else if (const auto* moved = std::get_if<cgpui::PointerMoved>(&event);
                   moved != nullptr) {
          last_pointer_position = moved->position;
        }
      });
  if (!window) {
    return 2;
  }
  observed_window = window->get();
  const auto surface =
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface());
  const auto hwnd = static_cast<HWND>(surface.hwnd);
  if (hwnd == nullptr) {
    return 3;
  }

  const auto initial = (*window)->pointer_capture_state();
  if (!initial.supported || initial.captured) {
    return 4;
  }
  (*window)->set_pointer_capture(true);
  if (GetCapture() != hwnd || !(*window)->pointer_capture_state().captured) {
    return 5;
  }
  SendMessageW(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(-12, 240));
  if (last_pointer_position.x != -12.0F || last_pointer_position.y != 240.0F) {
    return 6;
  }

  const HWND other = CreateWindowExW(
      0, L"STATIC", L"capture owner", 0, 0, 0, 1, 1,
      HWND_MESSAGE, nullptr, GetModuleHandleW(nullptr), nullptr);
  if (other == nullptr) {
    return 7;
  }
  SetCapture(other);
  if (GetCapture() != other || capture_lost_count != 1 ||
      !loss_saw_released_state) {
    DestroyWindow(other);
    return 8;
  }
  (*window)->set_pointer_capture(false);
  if (GetCapture() != other) {
    DestroyWindow(other);
    return 9;
  }

  (*window)->set_pointer_capture(true);
  SendMessageW(hwnd, WM_CANCELMODE, 0, 0);
  if (GetCapture() != nullptr || capture_lost_count != 2 ||
      (*window)->pointer_capture_state().captured) {
    DestroyWindow(other);
    return 10;
  }
  DestroyWindow(other);
  return 0;
}
