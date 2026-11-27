#pragma once

// Screen read back for DirectComposition tests: what DWM composited, in
// physical pixels (the tests are per-monitor DPI aware).
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "cgpui/renderer/renderer_frame_pixels.hpp"
#include "../support/win32_desktop_test_support.hpp"

#include <array>
#include <cstdlib>
#include <iostream>

namespace cgpui_test {

struct ScreenColor {
  int r = 0;
  int g = 0;
  int b = 0;
};

inline void pump_messages_for(DWORD milliseconds) {
  const ULONGLONG end = GetTickCount64() + milliseconds;
  do {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    Sleep(10);
  } while (GetTickCount64() < end);
}

// Polls the screen at `point` for up to two seconds until it shows
// `expected` within 12 per channel; prints what it read.
inline bool expect_screen_color(ScreenColor expected, POINT point) {
  ScreenColor seen{};
  const ULONGLONG end = GetTickCount64() + 2000;
  bool matched = false;
  do {
    pump_messages_for(30);
    HDC screen = GetDC(nullptr);
    const COLORREF color = GetPixel(screen, point.x, point.y);
    ReleaseDC(nullptr, screen);
    seen = {GetRValue(color), GetGValue(color), GetBValue(color)};
    matched = color != CLR_INVALID && std::abs(seen.r - expected.r) <= 12 &&
              std::abs(seen.g - expected.g) <= 12 && std::abs(seen.b - expected.b) <= 12;
  } while (!matched && GetTickCount64() < end);
  std::cout << "screen " << point.x << "," << point.y << ": " << seen.r << ' ' << seen.g
            << ' ' << seen.b << " (expected " << expected.r << ' ' << expected.g << ' '
            << expected.b << ")" << (matched ? "" : " MISMATCH") << '\n';
  return matched;
}

// A captured RGBA pixel within 2 per channel of `expected`; prints it.
inline bool expect_texture_pixel(const cgpui::RendererFramePixels& pixels, std::uint32_t x,
                                 std::uint32_t y, std::array<int, 4> expected) {
  const auto pixel = pixels.pixel_rgba8(x, y);
  if (!pixel) return false;
  std::cout << "texture " << x << "," << y << ": " << int((*pixel)[0]) << ' '
            << int((*pixel)[1]) << ' ' << int((*pixel)[2]) << ' ' << int((*pixel)[3]) << '\n';
  for (std::size_t channel = 0; channel < 4; ++channel) {
    if (std::abs(int((*pixel)[channel]) - expected[channel]) > 2) return false;
  }
  return true;
}

// `x`, `y` pixels into the primary monitor's work area.
inline POINT work_area_point(int x, int y) {
  RECT work{};
  SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
  return POINT{work.left + x, work.top + y};
}

// A topmost green 96 x 96 window and, 16 pixels inside it, an owned
// 64 x 64 window without a redirection surface for DirectComposition.
class CompositionTestWindows {
 public:
  CompositionTestWindows(int x, int y) : origin_(work_area_point(x, y)) {
    if (!desktop_.ready()) return;
    const HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSEXW backdrop_class{.cbSize = sizeof(WNDCLASSEXW), .lpfnWndProc = DefWindowProcW,
                               .hInstance = instance,
                               .hbrBackground = CreateSolidBrush(RGB(0, 255, 0)),
                               .lpszClassName = L"CGPUICompositionTestBackdrop"};
    WNDCLASSEXW popup_class{.cbSize = sizeof(WNDCLASSEXW), .lpfnWndProc = DefWindowProcW,
                            .hInstance = instance,
                            .lpszClassName = L"CGPUICompositionTestPopup"};
    RegisterClassExW(&backdrop_class);
    RegisterClassExW(&popup_class);
    backdrop_ = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, backdrop_class.lpszClassName,
                                L"", WS_POPUP | WS_VISIBLE, origin_.x, origin_.y, 96, 96,
                                nullptr, nullptr, instance, nullptr);
    popup_ = CreateWindowExW(
        WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP,
        popup_class.lpszClassName, L"", WS_POPUP, origin_.x + 16, origin_.y + 16, 64, 64,
        backdrop_, nullptr, instance, nullptr);
    if (popup_ != nullptr) ShowWindow(popup_, SW_SHOWNOACTIVATE);
    pump_messages_for(200);
  }
  ~CompositionTestWindows() {
    if (popup_ != nullptr) DestroyWindow(popup_);
    if (backdrop_ != nullptr) DestroyWindow(backdrop_);
  }
  CompositionTestWindows(const CompositionTestWindows&) = delete;
  CompositionTestWindows& operator=(const CompositionTestWindows&) = delete;

  [[nodiscard]] bool valid() const { return backdrop_ != nullptr && popup_ != nullptr; }
  [[nodiscard]] HWND popup() const { return popup_; }
  [[nodiscard]] POINT backdrop_point() const { return {origin_.x + 8, origin_.y + 8}; }
  [[nodiscard]] POINT popup_point(int x, int y) const {
    return {origin_.x + 16 + x, origin_.y + 16 + y};
  }
  [[nodiscard]] bool expect(ScreenColor color, POINT point) const {
    return expect_screen_color(color, point);
  }

 private:
  Win32DesktopTestLease desktop_;
  POINT origin_{};
  HWND backdrop_ = nullptr;
  HWND popup_ = nullptr;
};

} // namespace cgpui_test
