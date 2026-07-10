#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/native_surface.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui_test {

inline void pump_messages() {
  MSG message{};
  while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
}

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

    hwnd_ = CreateWindowExW(
        0,
        class_name,
        title,
        WS_POPUP,
        64,
        64,
        64,
        64,
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

  [[nodiscard]] cgpui::Size resize_client(int width, int height) const {
    SetWindowPos(
        hwnd_,
        nullptr,
        0,
        0,
        width,
        height,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    UpdateWindow(hwnd_);
    pump_messages();
    Sleep(50);
    return framebuffer_size();
  }

 private:
  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
};

} // namespace cgpui_test
