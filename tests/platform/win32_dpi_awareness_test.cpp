#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cmath>
#include <iostream>
#include <variant>

namespace {

int fail(int code, const char* message) {
  std::cerr << "win32 dpi awareness: " << message << '\n';
  return code;
}

} // namespace

int main() {
  // Nothing in this process sets awareness before the framework does.
  if (GetAwarenessFromDpiAwarenessContext(GetThreadDpiAwarenessContext()) !=
      DPI_AWARENESS_UNAWARE) {
    return fail(1, "test process started with a non-default awareness");
  }

  auto app = cgpui::create_platform_application();
  if (!app) return fail(2, "create_platform_application failed");

  const cgpui::PlatformDpiAwarenessState state = (*app)->dpi_awareness();
  if (state.backend != "win32" || !state.declared_by_framework ||
      state.already_set_by_process ||
      state.awareness != cgpui::PlatformDpiAwareness::per_monitor_aware_v2) {
    return fail(3, "application did not declare per-monitor v2 awareness");
  }

  float last_scale = 0.0F;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 DPI Awareness Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* resized = std::get_if<cgpui::WindowResized>(&event)) {
          last_scale = resized->scale.value;
        }
      });
  if (!window) return fail(4, "create_window failed");
  auto* hwnd = static_cast<HWND>(
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface()).hwnd);

  const DPI_AWARENESS_CONTEXT window_context = GetWindowDpiAwarenessContext(hwnd);
  if (GetAwarenessFromDpiAwarenessContext(window_context) !=
          DPI_AWARENESS_PER_MONITOR_AWARE ||
      !AreDpiAwarenessContextsEqual(
          window_context, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) ||
      !AreDpiAwarenessContextsEqual(
          GetThreadDpiAwarenessContext(),
          DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
    return fail(5, "window is not per-monitor v2 aware");
  }

  // The reported scale follows the monitor DPI rather than the 96 DPI that
  // virtualized (unaware) windows always see.
  const float monitor_scale = static_cast<float>(GetDpiForWindow(hwnd)) / 96.0F;
  if (std::fabs((*window)->state().scale.value - monitor_scale) > 0.01F) {
    return fail(6, "window scale does not match GetDpiForWindow");
  }

  // A second declaration is a cached no-op, and the existing WM_DPICHANGED
  // path still reaches WindowResized under per-monitor v2.
  auto second = cgpui::create_platform_application();
  if (!second || (*second)->dpi_awareness().awareness !=
                     cgpui::PlatformDpiAwareness::per_monitor_aware_v2) {
    return fail(7, "second application changed the awareness state");
  }
  const unsigned int dpi = monitor_scale > 1.4F ? 96U : 144U;
  RECT suggested{};
  GetWindowRect(hwnd, &suggested);
  SendMessageW(hwnd, WM_DPICHANGED, MAKEWPARAM(dpi, dpi),
               reinterpret_cast<LPARAM>(&suggested));
  if (std::fabs(last_scale - static_cast<float>(dpi) / 96.0F) > 0.01F) {
    return fail(8, "WM_DPICHANGED did not update the window scale");
  }
  return 0;
}
