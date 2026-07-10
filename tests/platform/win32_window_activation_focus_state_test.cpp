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
  bool activated = false;
  bool deactivated = false;
  bool focused = false;
  bool blurred = false;
  bool activation_state_matched = false;
  bool deactivation_state_matched = false;
  bool focus_state_matched = false;
  bool blur_state_matched = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Activation Focus State Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* activation = std::get_if<cgpui::WindowActivated>(&event);
            activation != nullptr && observed_window != nullptr) {
          const auto lifecycle = observed_window->lifecycle_state();
          activated = activated || activation->active;
          deactivated = deactivated || !activation->active;
          activation_state_matched =
              activation_state_matched ||
              (activation->active && lifecycle.active);
          deactivation_state_matched =
              deactivation_state_matched ||
              (!activation->active && !lifecycle.active);
        }
        if (const auto* focus = std::get_if<cgpui::WindowFocused>(&event);
            focus != nullptr && observed_window != nullptr) {
          const auto lifecycle = observed_window->lifecycle_state();
          focused = focused || focus->focused;
          blurred = blurred || !focus->focused;
          focus_state_matched =
              focus_state_matched || (focus->focused && lifecycle.focused);
          blur_state_matched =
              blur_state_matched || (!focus->focused && !lifecycle.focused);
        }
      });
  if (!window) {
    return 2;
  }
  observed_window = window->get();

  const auto surface =
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface());
  auto* hwnd = static_cast<HWND>(surface.hwnd);
  if (hwnd == nullptr) {
    return 3;
  }

  SendMessageW(hwnd, WM_ACTIVATE, WA_ACTIVE, 0);
  SendMessageW(hwnd, WM_SETFOCUS, 0, 0);
  SendMessageW(hwnd, WM_KILLFOCUS, 0, 0);
  SendMessageW(hwnd, WM_ACTIVATE, WA_INACTIVE, 0);

  if (!activated || !deactivated || !focused || !blurred) {
    return 4;
  }
  if (!activation_state_matched || !deactivation_state_matched ||
      !focus_state_matched || !blur_state_matched) {
    return 5;
  }
  const auto final = (*window)->lifecycle_state();
  if (final.active || final.focused) {
    return 6;
  }
  return 0;
}
