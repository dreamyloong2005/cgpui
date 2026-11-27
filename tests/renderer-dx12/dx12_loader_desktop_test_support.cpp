#include "dx12_loader_desktop_test_support.hpp"
#include "../support/win32_desktop_test_support.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>

namespace cgpui_test {
unsigned long loader_wait_for_child(void* process, void* desktop_ready) {
  if (desktop_ready) {
    const HANDLE signals[]{desktop_ready, process};
    const DWORD queued = WaitForMultipleObjects(2, signals, FALSE,
        desktop_test_queue_timeout_ms + 1000);
    if (queued != WAIT_OBJECT_0) {
      std::fprintf(stderr, "loader visible child did not acquire desktop: wait=%lu\n", queued);
      return queued == WAIT_TIMEOUT ? WAIT_TIMEOUT : WAIT_FAILED;
    }
  }
  return WaitForSingleObject(process, 20000);
}
} // namespace cgpui_test
