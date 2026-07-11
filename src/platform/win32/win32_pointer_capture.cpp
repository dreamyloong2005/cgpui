#include "win32_pointer_capture_internal.hpp"

namespace cgpui {

bool win32_pointer_captured(HWND hwnd) {
  return hwnd != nullptr && GetCapture() == hwnd;
}

void capture_win32_pointer(HWND hwnd) {
  if (hwnd != nullptr && GetCapture() != hwnd) {
    SetCapture(hwnd);
  }
}

bool release_win32_pointer(HWND hwnd) {
  if (!win32_pointer_captured(hwnd)) {
    return false;
  }
  return ReleaseCapture() != FALSE;
}

} // namespace cgpui
