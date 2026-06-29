#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cmath>
#include <variant>

namespace {

bool nearly_equal(float lhs, float rhs) {
  return std::fabs(lhs - rhs) < 0.01F;
}

unsigned int alternate_dpi_for(float current_scale) {
  const unsigned int high_dpi = 144U;
  const unsigned int low_dpi = 96U;
  return nearly_equal(current_scale, static_cast<float>(high_dpi) / 96.0F)
             ? low_dpi
             : high_dpi;
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 2;
  }

  bool saw_matching_scale = false;
  float expected_scale = 0.0F;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 DPI Scale Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* resized = std::get_if<cgpui::WindowResized>(&event);
            resized != nullptr && expected_scale > 0.0F &&
            nearly_equal(resized->scale.value, expected_scale)) {
          saw_matching_scale = true;
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

  const cgpui::WindowState initial_state = (*window)->state();
  const unsigned int dpi = alternate_dpi_for(initial_state.scale.value);
  expected_scale = static_cast<float>(dpi) / 96.0F;

  RECT suggested_rect{};
  GetWindowRect(hwnd, &suggested_rect);
  SendMessageW(
      hwnd,
      WM_DPICHANGED,
      MAKEWPARAM(dpi, dpi),
      reinterpret_cast<LPARAM>(&suggested_rect));

  const cgpui::WindowState updated_state = (*window)->state();
  if (!saw_matching_scale) {
    return 5;
  }
  if (!nearly_equal(updated_state.scale.value, expected_scale)) {
    return 6;
  }

  return 0;
}
