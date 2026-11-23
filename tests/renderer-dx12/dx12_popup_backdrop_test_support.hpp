#pragma once

#include "dx12_popup_test_support.hpp"

namespace cgpui_test {

// These tests inspect the public capture at larger framebuffer extents than
// the desktop alpha tracer. Keep their native composition popup hidden so a
// wide Gaussian oracle does not obscure unrelated desktop windows.
struct Dx12BackdropPopupWindow {
  HWND hwnd = CreateWindowExW(
      WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP,
      L"STATIC", L"CGPUI DX12 popup backdrop acceptance", WS_POPUP,
      0, 0, 64, 64, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  ~Dx12BackdropPopupWindow() { if (hwnd) DestroyWindow(hwnd); }
  Dx12BackdropPopupWindow() = default;
  Dx12BackdropPopupWindow(const Dx12BackdropPopupWindow&) = delete;
  Dx12BackdropPopupWindow& operator=(const Dx12BackdropPopupWindow&) = delete;
};

} // namespace cgpui_test
