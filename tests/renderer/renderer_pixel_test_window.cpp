#include "renderer_pixel_test_window.hpp"

#include <cstdio>

namespace cgpui_test {
void pump_messages() {
  MSG message{};
  while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
}

RendererPixelTestWindow::RendererPixelTestWindow(
    const wchar_t* class_name, const wchar_t* title)
    : instance_(GetModuleHandleW(nullptr)) {
  if (!desktop_.ready()) return;
  WNDCLASSEXW window_class{};
  window_class.cbSize = sizeof(WNDCLASSEXW);
  window_class.lpfnWndProc = DefWindowProcW;
  window_class.hInstance = instance_;
  window_class.lpszClassName = class_name;
  RegisterClassExW(&window_class);
  hwnd_ = CreateWindowExW(0, class_name, title, WS_POPUP,
      64, 64, 64, 64, nullptr, nullptr, instance_, nullptr);
  if (hwnd_ != nullptr) {
    ShowWindow(hwnd_, SW_SHOWNA);
    UpdateWindow(hwnd_);
  } else {
    std::fprintf(stderr, "pixel test CreateWindowExW failed: %lu\n", GetLastError());
  }
}

RendererPixelTestWindow::~RendererPixelTestWindow() {
  if (hwnd_ != nullptr) DestroyWindow(hwnd_);
}
bool RendererPixelTestWindow::valid() const {
  return instance_ != nullptr && hwnd_ != nullptr;
}
cgpui::Win32SurfaceHandle RendererPixelTestWindow::surface() const {
  return {.hinstance = instance_, .hwnd = hwnd_};
}
cgpui::Size RendererPixelTestWindow::framebuffer_size() const {
  RECT rect{};
  if (!GetClientRect(hwnd_, &rect)) {
    std::fprintf(stderr, "pixel test GetClientRect failed: %lu\n", GetLastError());
    return {};
  }
  return {static_cast<float>(rect.right - rect.left),
          static_cast<float>(rect.bottom - rect.top)};
}
cgpui::Size RendererPixelTestWindow::resize_client(int width, int height) const {
  if (!SetWindowPos(hwnd_, nullptr, 0, 0, width, height,
                   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE)) {
    std::fprintf(stderr, "pixel test SetWindowPos failed: %lu\n", GetLastError());
    return {};
  }
  UpdateWindow(hwnd_);
  pump_messages();
  Sleep(50);
  return framebuffer_size();
}
} // namespace cgpui_test
