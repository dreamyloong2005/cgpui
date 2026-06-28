#include "cgpui/renderer/renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <iostream>

namespace {

constexpr int kClientWidth = 160;
constexpr int kClientHeight = 120;

void pump_messages() {
  MSG message{};
  while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
}

class VisibleWindow {
 public:
  VisibleWindow() : instance_(GetModuleHandleW(nullptr)) {
    const wchar_t* class_name = L"CGPUIVulkanSolidRectTestWindow";

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.lpfnWndProc = DefWindowProcW;
    window_class.hInstance = instance_;
    window_class.lpszClassName = class_name;
    RegisterClassExW(&window_class);

    RECT window_rect{0, 0, kClientWidth, kClientHeight};
    AdjustWindowRectEx(&window_rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
    const int window_width = window_rect.right - window_rect.left;
    const int window_height = window_rect.bottom - window_rect.top;

    hwnd_ = CreateWindowExW(
        0,
        class_name,
        L"CGPUI Vulkan Solid Rect Test",
        WS_OVERLAPPEDWINDOW,
        64,
        64,
        window_width,
        window_height,
        nullptr,
        nullptr,
        instance_,
        nullptr);
    if (hwnd_ != nullptr) {
      ShowWindow(hwnd_, SW_SHOW);
      SetWindowPos(
          hwnd_,
          HWND_TOPMOST,
          64,
          64,
          window_width,
          window_height,
          SWP_SHOWWINDOW);
      UpdateWindow(hwnd_);
      SetForegroundWindow(hwnd_);
      pump_messages();
      Sleep(100);
    }
  }

  ~VisibleWindow() {
    if (hwnd_ != nullptr) {
      DestroyWindow(hwnd_);
    }
  }

  [[nodiscard]] bool valid() const {
    return instance_ != nullptr && hwnd_ != nullptr;
  }

  [[nodiscard]] cgpui::Win32SurfaceHandle surface() const {
    return cgpui::Win32SurfaceHandle{.hinstance = instance_, .hwnd = hwnd_};
  }

  [[nodiscard]] cgpui::Size framebuffer_size() const {
    RECT client_rect{};
    GetClientRect(hwnd_, &client_rect);
    return cgpui::Size{
        .width = static_cast<float>(client_rect.right - client_rect.left),
        .height = static_cast<float>(client_rect.bottom - client_rect.top),
    };
  }

  [[nodiscard]] bool sample_center(COLORREF& color) const {
    RECT client_rect{};
    if (GetClientRect(hwnd_, &client_rect) == 0) {
      return false;
    }

    POINT center{
        .x = (client_rect.right - client_rect.left) / 2,
        .y = (client_rect.bottom - client_rect.top) / 2,
    };
    if (ClientToScreen(hwnd_, &center) == 0) {
      return false;
    }

    HDC screen = GetDC(nullptr);
    if (screen == nullptr) {
      return false;
    }
    color = GetPixel(screen, center.x, center.y);
    ReleaseDC(nullptr, screen);
    return color != CLR_INVALID;
  }

 private:
  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
};

bool is_blue_dominant(COLORREF color) {
  const auto red = static_cast<int>(GetRValue(color));
  const auto green = static_cast<int>(GetGValue(color));
  const auto blue = static_cast<int>(GetBValue(color));
  return blue >= green + 25 && green >= red + 40;
}

} // namespace

int main() {
  SetProcessDPIAware();
  VisibleWindow window;
  if (!window.valid()) {
    return 4;
  }

  const cgpui::Size framebuffer_size = window.framebuffer_size();
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = framebuffer_size,
      .scale = cgpui::DpiScale{1.0F}});
  if (!renderer) {
    std::cerr << renderer.error().message << '\n';
    return 1;
  }

  auto frame = (*renderer)->begin_frame();
  if (!frame) {
    std::cerr << frame.error().message << '\n';
    return 2;
  }

  (*frame)->clear(cgpui::Color{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F});
  (*frame)->draw_rect(cgpui::SolidRect{
      .rect =
          cgpui::Rect{
              .origin =
                  cgpui::Point{
                      .x = framebuffer_size.width * 0.10F,
                      .y = framebuffer_size.height * 0.10F,
                  },
              .size =
                  cgpui::Size{
                      .width = framebuffer_size.width * 0.80F,
                      .height = framebuffer_size.height * 0.80F,
                  },
          },
      .color = cgpui::Color{.r = 0.23F, .g = 0.55F, .b = 0.86F, .a = 1.0F},
  });

  if (auto presented = (*frame)->present(); !presented) {
    std::cerr << presented.error().message << '\n';
    return 3;
  }

  COLORREF sampled = CLR_INVALID;
  for (int attempt = 0; attempt < 10; ++attempt) {
    pump_messages();
    Sleep(50);
    if (window.sample_center(sampled) && is_blue_dominant(sampled)) {
      return 0;
    }
  }

  if (sampled != CLR_INVALID) {
    std::cerr << "Expected blue-dominant center pixel, got RGB("
              << static_cast<int>(GetRValue(sampled)) << ", "
              << static_cast<int>(GetGValue(sampled)) << ", "
              << static_cast<int>(GetBValue(sampled)) << ")\n";
  }
  return 5;
}
