#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cmath>
#include <variant>

namespace {

bool nearly_equal(float lhs, float rhs) {
  return std::fabs(lhs - rhs) < 0.01F;
}

bool size_equal(cgpui::Size lhs, cgpui::Size rhs) {
  return nearly_equal(lhs.width, rhs.width) &&
      nearly_equal(lhs.height, rhs.height);
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }

  cgpui::PlatformWindow* observed_window = nullptr;
  bool resize_state_matched = false;
  bool scale_state_matched = false;
  float expected_scale = 0.0F;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Resize Scale State Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        const auto* resized = std::get_if<cgpui::WindowResized>(&event);
        if (resized == nullptr || observed_window == nullptr) {
          return;
        }
        const cgpui::WindowState state = observed_window->state();
        resize_state_matched = resize_state_matched ||
            (size_equal(resized->size, state.framebuffer_size) &&
             resized->size.width > 320.0F && resized->size.height > 240.0F);
        scale_state_matched = scale_state_matched ||
            (expected_scale > 0.0F &&
             nearly_equal(resized->scale.value, expected_scale) &&
             nearly_equal(state.scale.value, expected_scale));
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

  RECT rect{};
  GetWindowRect(hwnd, &rect);
  SetWindowPos(
      hwnd,
      nullptr,
      rect.left,
      rect.top,
      (rect.right - rect.left) + 80,
      (rect.bottom - rect.top) + 60,
      SWP_NOZORDER | SWP_NOACTIVATE);

  const unsigned int dpi =
      nearly_equal((*window)->state().scale.value, 1.5F) ? 96U : 144U;
  expected_scale = static_cast<float>(dpi) / 96.0F;
  GetWindowRect(hwnd, &rect);
  SendMessageW(
      hwnd,
      WM_DPICHANGED,
      MAKEWPARAM(dpi, dpi),
      reinterpret_cast<LPARAM>(&rect));

  if (!resize_state_matched) {
    return 4;
  }
  if (!scale_state_matched) {
    return 5;
  }
  return 0;
}
