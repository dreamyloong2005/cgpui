#pragma once

#include "dx12_recovery_test_support.hpp"
#include "../renderer/win32_composition_screen_support.hpp"

namespace cgpui_test {

inline bool dx12_popup_desktop_ready() {
  // This test samples physical screen pixels, not a DPI-virtualized client DC.
  if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) &&
      !AreDpiAwarenessContextsEqual(GetThreadDpiAwarenessContext(),
                                   DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) return false;
  RECT work{};
  return SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0) &&
      work.right - work.left >= 192 && work.bottom - work.top >= 192;
}

inline cgpui::RenderSurfaceDescriptor dx12_popup_descriptor(HWND hwnd) {
  return {.native_surface = cgpui::Win32SurfaceHandle{
              .hinstance = GetModuleHandleW(nullptr), .hwnd = hwnd, .composition = true},
      .framebuffer_size = {64, 64}, .scale = {1}, .transparent_background = true};
}

} // namespace cgpui_test
