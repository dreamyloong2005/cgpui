#include "win32_window_proc_popup_internal.hpp"
#include "win32_popup_window_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_popup(HWND hwnd, UINT message, LRESULT& result) {
  switch (message) {
    case WM_MOUSEACTIVATE:
      if (!win32_is_popup_window(hwnd)) return false;
      result = MA_NOACTIVATE;
      return true;
    case WM_MOVE:
      if (!win32_is_popup_window(hwnd)) win32_move_owned_popups(hwnd);
      return false;
    default:
      return false;
  }
}

} // namespace cgpui
