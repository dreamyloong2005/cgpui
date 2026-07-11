#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace cgpui::test {

class Win32ClipboardTestLock {
 public:
  Win32ClipboardTestLock()
      : handle_(CreateMutexW(
            nullptr,
            FALSE,
            L"Local\\cgpui-system-clipboard-tests")) {
    if (handle_ == nullptr) return;
    const DWORD result = WaitForSingleObject(handle_, 30000U);
    owns_ = result == WAIT_OBJECT_0 || result == WAIT_ABANDONED;
  }

  ~Win32ClipboardTestLock() {
    if (owns_) ReleaseMutex(handle_);
    if (handle_ != nullptr) CloseHandle(handle_);
  }

  Win32ClipboardTestLock(const Win32ClipboardTestLock&) = delete;
  Win32ClipboardTestLock& operator=(const Win32ClipboardTestLock&) = delete;

  [[nodiscard]] explicit operator bool() const { return owns_; }

 private:
  HANDLE handle_ = nullptr;
  bool owns_ = false;
};

} // namespace cgpui::test
