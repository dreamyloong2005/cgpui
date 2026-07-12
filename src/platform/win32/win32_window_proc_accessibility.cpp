#include "win32_window_proc_accessibility_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_accessibility(
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32AccessibilityMessageTarget* window,
    LRESULT& result) {
  return message == WM_GETOBJECT && window != nullptr &&
      window->accessibility_object(wparam, lparam, result);
}

} // namespace cgpui
