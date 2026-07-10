#pragma once

#include "cgpui/renderer/renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <memory>

namespace cgpui_test {

class VulkanPixelTestWindow {
 public:
  VulkanPixelTestWindow(const wchar_t* class_name, const wchar_t* title)
      : instance_(GetModuleHandleW(nullptr)) {
    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.lpfnWndProc = DefWindowProcW;
    window_class.hInstance = instance_;
    window_class.lpszClassName = class_name;
    RegisterClassExW(&window_class);

    RECT window_rect{0, 0, 64, 64};
    AdjustWindowRectEx(&window_rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
    hwnd_ = CreateWindowExW(
        0,
        class_name,
        title,
        WS_OVERLAPPEDWINDOW,
        64,
        64,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        nullptr,
        nullptr,
        instance_,
        nullptr);
    if (hwnd_ != nullptr) {
      ShowWindow(hwnd_, SW_SHOWNA);
      UpdateWindow(hwnd_);
    }
  }

  ~VulkanPixelTestWindow() {
    if (hwnd_ != nullptr) {
      DestroyWindow(hwnd_);
    }
  }

  VulkanPixelTestWindow(const VulkanPixelTestWindow&) = delete;
  VulkanPixelTestWindow& operator=(const VulkanPixelTestWindow&) = delete;

  [[nodiscard]] bool valid() const {
    return instance_ != nullptr && hwnd_ != nullptr;
  }

  [[nodiscard]] cgpui::Win32SurfaceHandle surface() const {
    return {.hinstance = instance_, .hwnd = hwnd_};
  }

  [[nodiscard]] cgpui::Size framebuffer_size() const {
    RECT client_rect{};
    GetClientRect(hwnd_, &client_rect);
    return {
        .width = static_cast<float>(client_rect.right - client_rect.left),
        .height = static_cast<float>(client_rect.bottom - client_rect.top),
    };
  }

 private:
  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
};

inline bool pixel_near(
    const cgpui::RendererFramePixels& pixels,
    std::uint32_t x,
    std::uint32_t y,
    std::array<std::uint8_t, 4> expected,
    std::uint8_t tolerance = 1) {
  const auto actual = pixels.pixel_rgba8(x, y);
  if (!actual.has_value()) {
    return false;
  }
  for (std::size_t index = 0; index < actual->size(); ++index) {
    if (std::abs(static_cast<int>((*actual)[index]) - expected[index]) >
        tolerance) {
      return false;
    }
  }
  return true;
}

} // namespace cgpui_test
