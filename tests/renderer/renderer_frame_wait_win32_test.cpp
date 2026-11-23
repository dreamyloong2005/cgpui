#include "../support/frame_wait_win32_test_support.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <iostream>

namespace {
unsigned deliveries = 0;
LRESULT CALLBACK messages(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == WM_APP + 19) { ++deliveries; return 0; }
  return DefWindowProcW(window, message, wparam, lparam);
}
}

int main() {
  const auto instance = GetModuleHandleW(nullptr);
  WNDCLASSW type{};
  type.lpfnWndProc = messages;
  type.hInstance = instance;
  type.lpszClassName = L"CGPUIFrameWaitMessageControl";
  if (!RegisterClassW(&type)) return 1;
  const HWND window = CreateWindowExW(0, type.lpszClassName, L"", 0,
      0, 0, 0, 0, HWND_MESSAGE, nullptr, instance, nullptr);
  if (!window) return 2;
  for (unsigned index = 0; index != 40; ++index)
    if (!PostMessageW(window, WM_APP + 19, 0, 0)) return 3;
  cgpui_test::detail::pump_frame_wait_win32_messages(nullptr, 1000);
  if (deliveries != 32) return 4;
  cgpui_test::detail::pump_frame_wait_win32_messages(nullptr, 3);
  if (deliveries != 35) return 5;
  cgpui_test::detail::pump_frame_wait_win32_messages(nullptr, 0);
  if (deliveries != 35) return 6;
  cgpui_test::detail::pump_frame_wait_win32_messages(nullptr, 32);
  if (deliveries != 40) return 7;
  PostQuitMessage(37);
  cgpui_test::detail::pump_frame_wait_win32_messages(nullptr, 32);
  MSG quit{};
  if (!PeekMessageW(&quit, nullptr, WM_QUIT, WM_QUIT, PM_REMOVE) || quit.wParam != 37) return 8;
  DestroyWindow(window);
  if (!UnregisterClassW(type.lpszClassName, instance)) return 9;
  std::cout << "FRAME_WAIT_NATIVE dispatch-budget=32 smaller/zero budgets and quit-code=37 PASS\n";
  return 0;
}
