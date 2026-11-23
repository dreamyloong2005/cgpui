#include "dx12_composition_probe_windows_test_support.hpp"
#include <cstdio>
#include <cstdlib>

namespace cgpui_test {
namespace {
constexpr const wchar_t* backdrop_name = L"CGPUICompositionPacingProbeBackdrop";
constexpr const wchar_t* popup_name = L"CGPUICompositionPacingProbePopup";
}
bool composition_probe_dispatch() {
  MSG message{};
  for (unsigned count = 0; count != 32 && PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE); ++count) {
    if (message.message == WM_QUIT) {
      PostQuitMessage(static_cast<int>(message.wParam));
      return false;
    }
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
  return true;
}
Dx12CompositionProbeWindows::Dx12CompositionProbeWindows() = default;
Dx12CompositionProbeWindows::~Dx12CompositionProbeWindows() {
  if (popup_) DestroyWindow(popup_);
  if (backdrop_) DestroyWindow(backdrop_);
  const auto instance = GetModuleHandleW(nullptr);
  if (popup_class_) UnregisterClassW(popup_name, instance);
  if (backdrop_class_) UnregisterClassW(backdrop_name, instance);
  if (brush_) DeleteObject(brush_);
}
bool Dx12CompositionProbeWindows::open(bool visible) {
  visible_ = visible;
  if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) &&
      !AreDpiAwarenessContextsEqual(GetThreadDpiAwarenessContext(),
                                   DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) return false;
  RECT work{};
  if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0) ||
      work.right - work.left < 224 || work.bottom - work.top < 224) return false;
  origin_ = {work.left + 32, work.top + 32};
  const auto instance = GetModuleHandleW(nullptr);
  brush_ = CreateSolidBrush(RGB(0, 255, 0));
  WNDCLASSEXW backdrop{.cbSize = sizeof(WNDCLASSEXW), .lpfnWndProc = DefWindowProcW,
      .hInstance = instance, .hbrBackground = brush_, .lpszClassName = backdrop_name};
  WNDCLASSEXW popup{.cbSize = sizeof(WNDCLASSEXW), .lpfnWndProc = DefWindowProcW,
      .hInstance = instance, .lpszClassName = popup_name};
  backdrop_class_ = RegisterClassExW(&backdrop);
  popup_class_ = RegisterClassExW(&popup);
  if (!brush_ || !backdrop_class_ || !popup_class_) return false;
  backdrop_ = CreateWindowExW(WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
      backdrop_name, L"composition pacing experiment", WS_POPUP, origin_.x, origin_.y,
      160, 160, nullptr, nullptr, instance, nullptr);
  popup_ = CreateWindowExW(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP,
      popup_name, L"", WS_POPUP, origin_.x + 16, origin_.y + 16, 64, 64,
      backdrop_, nullptr, instance, nullptr);
  if (!backdrop_ || !popup_) return false;
  // The initially hidden branch never shows either HWND, even transiently.
  if (visible_) {
    ShowWindow(backdrop_, SW_SHOWNOACTIVATE);
    UpdateWindow(backdrop_);
    ShowWindow(popup_, SW_SHOWNOACTIVATE);
  }
  return visibility_unchanged();
}
bool Dx12CompositionProbeWindows::visibility_unchanged() const {
  return popup_ && backdrop_ && IsWindow(popup_) &&
      bool(IsWindowVisible(popup_)) == visible_ && bool(IsWindowVisible(backdrop_)) == visible_ &&
      !IsIconic(popup_) && GetWindow(popup_, GW_OWNER) == backdrop_ &&
      (GetWindowLongPtrW(popup_, GWL_STYLE) & WS_POPUP) &&
      (GetWindowLongPtrW(popup_, GWL_EXSTYLE) & WS_EX_NOREDIRECTIONBITMAP);
}
HWND Dx12CompositionProbeWindows::popup() const { return popup_; }
bool Dx12CompositionProbeWindows::resize(UINT extent) {
  return SetWindowPos(popup_, nullptr, 0, 0, static_cast<int>(extent), static_cast<int>(extent),
      SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER) && visibility_unchanged();
}
bool Dx12CompositionProbeWindows::expect_screen(int r, int g, int b, int x, int y) const {
  if (!visible_ || !visibility_unchanged()) return false;
  const auto end = GetTickCount64() + 2000;
  COLORREF seen = CLR_INVALID;
  do {
    if (!composition_probe_dispatch() || GetTickCount64() >= end) break;
    const auto dc = GetDC(nullptr);
    if (dc) {
      seen = GetPixel(dc, origin_.x + 16 + x, origin_.y + 16 + y);
      ReleaseDC(nullptr, dc);
    }
    if (seen != CLR_INVALID && std::abs(int(GetRValue(seen)) - r) <= 12 &&
        std::abs(int(GetGValue(seen)) - g) <= 12 && std::abs(int(GetBValue(seen)) - b) <= 12)
      return true;
    Sleep(1);
  } while (GetTickCount64() < end);
  std::printf("COMPOSITION_PROBE screen_mismatch x=%d y=%d color=0x%08lX\n", x, y, seen);
  return false;
}
bool Dx12CompositionProbeWindows::expect_backdrop() const {
  return expect_screen(0, 255, 0, -8, -8);
}
} // namespace cgpui_test
