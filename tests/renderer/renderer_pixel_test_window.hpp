#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/native_surface.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui_test {
void pump_messages();

class RendererPixelTestWindow {
 public:
  RendererPixelTestWindow(const wchar_t* class_name, const wchar_t* title);
  ~RendererPixelTestWindow();
  RendererPixelTestWindow(const RendererPixelTestWindow&) = delete;
  RendererPixelTestWindow& operator=(const RendererPixelTestWindow&) = delete;
  [[nodiscard]] bool valid() const;
  [[nodiscard]] cgpui::Win32SurfaceHandle surface() const;
  [[nodiscard]] cgpui::Size framebuffer_size() const;
  [[nodiscard]] cgpui::Size resize_client(int width, int height) const;

 private:
  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
};
} // namespace cgpui_test
