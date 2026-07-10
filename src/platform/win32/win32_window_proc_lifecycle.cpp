#include "win32_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_lifecycle(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32WindowMessageTarget* window,
    LRESULT& result) {
  switch (message) {
    case WM_NCCREATE: {
      const auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
      auto* created_window =
          static_cast<Win32WindowMessageTarget*>(create->lpCreateParams);
      created_window->attach(hwnd);
      SetWindowLongPtrW(
          hwnd,
          GWLP_USERDATA,
          reinterpret_cast<LONG_PTR>(created_window));
      result = TRUE;
      return true;
    }
    case WM_SIZE:
      if (window != nullptr) {
        window->update_size();
      }
      result = 0;
      return true;
    case WM_DPICHANGED:
      if (window != nullptr) {
        window->dpi_changed(wparam, lparam);
      }
      result = 0;
      return true;
    case WM_ACTIVATE:
      if (window != nullptr) {
        window->activation_changed(LOWORD(wparam) != WA_INACTIVE);
      }
      result = 0;
      return true;
    case WM_SETFOCUS:
      if (window != nullptr) {
        window->focus_changed(true);
      }
      result = 0;
      return true;
    case WM_KILLFOCUS:
      if (window != nullptr) {
        window->focus_changed(false);
      }
      result = 0;
      return true;
    case WM_IME_STARTCOMPOSITION:
      if (window != nullptr) {
        window->ime_start_composition();
      }
      result = DefWindowProcW(hwnd, message, wparam, lparam);
      return true;
    case WM_IME_COMPOSITION:
      if (window != nullptr) {
        window->ime_composition(lparam);
      }
      result = DefWindowProcW(hwnd, message, wparam, lparam);
      return true;
    case WM_IME_ENDCOMPOSITION:
      if (window != nullptr) {
        window->ime_end_composition();
      }
      result = DefWindowProcW(hwnd, message, wparam, lparam);
      return true;
    case WM_CLOSE:
      if (window != nullptr) {
        window->close_requested();
      }
      result = 0;
      return true;
    case WM_PAINT: {
      PAINTSTRUCT paint{};
      BeginPaint(hwnd, &paint);
      EndPaint(hwnd, &paint);
      if (window != nullptr) {
        window->redraw_requested();
      }
      result = 0;
      return true;
    }
    case WM_NCDESTROY:
      if (window != nullptr) {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        window->detach();
      }
      result = DefWindowProcW(hwnd, message, wparam, lparam);
      return true;
    default:
      return false;
  }
}

} // namespace cgpui
