#include "vulkan_device_recovery_scene_test_support.hpp"
#include "win32_composition_screen_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) &&
      !AreDpiAwarenessContextsEqual(GetThreadDpiAwarenessContext(), DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
    return recovery_fail(8, "physical screen oracle requires per-monitor DPI awareness");
  VulkanRecoveryDeadline deadline;
  VulkanRecoveryHooks hooks;
  CompositionTestWindows window(560, 200);
  if (!deadline.valid() || !hooks.installed() || !window.valid()) return 1;
  if (!window.expect({0, 255, 0}, window.backdrop_point()))
    return recovery_fail(9, "desktop backdrop control is not visible before renderer creation");
  auto renderer = create_vulkan_renderer({
      .native_surface = Win32SurfaceHandle{.hinstance = GetModuleHandleW(nullptr),
          .hwnd = window.popup(), .composition = true},
      .framebuffer_size = {64, 64}, .scale = {1}, .transparent_background = true});
  if (!renderer || !(*renderer)->surface_composition().alpha_composited()) return 2;
  const auto red = recovery_image();
  const auto before = (*renderer)->backend_info();
  const auto baseline = recovery_scene_capture(**renderer, &red);
  if (!baseline || !recovery_scene_pixels(*baseline)) return 3;
  const auto initial_surfaces = hooks.observation().surfaces_created;
  auto frame = recovery_scene_frame(**renderer);
  if (!frame) return 4;
  hooks.arm(VulkanLossSite::submit);
  if (!recovery_retry((*frame)->present()) ||
      !recovery_native_cycle(before, (*renderer)->backend_info(), hooks.observation(), initial_surfaces) ||
      !(*renderer)->surface_composition().alpha_composited())
    return recovery_fail(5, "private interop popup did not recover on the same native capability");
  frame->reset();
  const auto restored = recovery_scene_capture(**renderer);
  if (!restored || !recovery_same_pixels(&*restored, *baseline))
    return recovery_fail(6, "private popup GPU retained pixels did not return");
  // Actual native D3D11/DComp presentation, not only offscreen reconstruction.
  // Black background and red image are opaque controls; existing popup tests
  // separately establish desktop per-pixel alpha/corners/shadows.
  if (!window.expect({0, 255, 0}, window.backdrop_point()) ||
      !window.expect({255, 0, 0}, window.popup_point(42, 42)) ||
      !window.expect({0, 0, 0}, window.popup_point(60, 60)))
    return recovery_fail(7, "reconstructed interop was not presented to desktop");
  return 0;
}
