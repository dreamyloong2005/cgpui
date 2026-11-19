#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace cgpui::test {

// Clipboard listeners (explorer.exe, the clipboard history service
// cbdhsvc, DefenderSessionHelper, sync tools) open the clipboard for tens of
// milliseconds after every change, so a direct OpenClipboard in a test
// helper can fail right after a write. Retry for up to one second.
[[nodiscard]] inline bool open_system_clipboard_for_test() {
  for (int attempt = 0; attempt < 100; ++attempt) {
    if (OpenClipboard(nullptr) != FALSE) return true;
    Sleep(10);
  }
  return false;
}

} // namespace cgpui::test
