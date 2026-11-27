#include "win32_desktop_test_support.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdio>
#include <cstdlib>

namespace cgpui_test {
Win32DesktopTestLease::Win32DesktopTestLease(DesktopTestAccess access)
    : required_(access == DesktopTestAccess::exclusive) {
  if (!required_) return;
  // Local names share this interactive session, not unrelated login sessions.
  // The handle is non-inheritable; child cases acquire their own explicit lease.
  mutex_ = CreateMutexW(nullptr, FALSE, L"Local\\CGPUI.VisibleDesktopTests.v1");
  if (!mutex_) {
    std::fprintf(stderr, "desktop test mutex creation failed: %lu\n", GetLastError());
    return;
  }
  const DWORD result = WaitForSingleObject(mutex_, desktop_test_queue_timeout_ms);
  // A crashed test has no surviving HWNDs. Windows transfers abandoned ownership.
  acquired_ = result == WAIT_OBJECT_0 || result == WAIT_ABANDONED;
  if (!acquired_)
    std::fprintf(stderr, "desktop test mutex wait failed: %lu (%lu)\n", result, GetLastError());
}

Win32DesktopTestLease::~Win32DesktopTestLease() {
  if (acquired_ && !ReleaseMutex(mutex_)) {
    std::fprintf(stderr, "desktop test lease released on the wrong thread: %lu\n", GetLastError());
    std::abort();
  }
  if (mutex_) CloseHandle(mutex_);
}

bool Win32DesktopTestLease::ready() const noexcept { return !required_ || acquired_; }
} // namespace cgpui_test
