#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui {

class Win32AccessibilityMessageTarget {
 public:
  virtual ~Win32AccessibilityMessageTarget() = default;
  virtual bool accessibility_object(
      WPARAM wparam,
      LPARAM lparam,
      LRESULT& result) = 0;
};

bool win32_window_proc_handle_accessibility(
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32AccessibilityMessageTarget* window,
    LRESULT& result);

} // namespace cgpui
