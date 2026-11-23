#pragma once

#include "cgpui/renderer/renderer_backend.hpp"
#include "cgpui/renderer/renderer.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <array>
#include <cmath>
#include <iostream>

namespace cgpui_test {

struct Dx12Window {
  HWND hwnd = CreateWindowExW(0, L"STATIC", L"CGPUI DX12 acceptance",
      WS_POPUP, 0, 0, 64, 64, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  ~Dx12Window() { if (hwnd) DestroyWindow(hwnd); }
  Dx12Window() = default;
  Dx12Window(const Dx12Window&) = delete;
  Dx12Window& operator=(const Dx12Window&) = delete;

  cgpui::RenderSurfaceDescriptor descriptor() const {
    return {.native_surface = cgpui::Win32SurfaceHandle{
                .hinstance = GetModuleHandleW(nullptr), .hwnd = hwnd},
            .framebuffer_size = {64, 64}, .scale = {1}};
  }
};

inline int dx12_fail(int code, const char* message) {
  std::cerr << "DX12 acceptance: " << message << '\n';
  return code;
}

inline bool dx12_pixel(const cgpui::RendererFramePixels* capture,
                       unsigned x, unsigned y,
                       std::array<unsigned char, 4> expected,
                       int tolerance = 1) {
  if (!capture || !capture->valid()) return false;
  const auto actual = capture->pixel_rgba8(x, y);
  if (!actual) return false;
  for (unsigned i = 0; i < 4; ++i) {
    if (std::abs(int((*actual)[i]) - int(expected[i])) > tolerance) {
      std::cerr << "pixel (" << x << ',' << y << ") channel " << i
                << ": got " << int((*actual)[i]) << ", expected "
                << int(expected[i]) << '\n';
      return false;
    }
  }
  return true;
}

} // namespace cgpui_test
