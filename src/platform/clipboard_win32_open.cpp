#include "clipboard_win32_internal.hpp"

#if defined(_WIN32)
#include <algorithm>

namespace cgpui {

// Clipboard listeners (explorer.exe, the clipboard history service cbdhsvc,
// DefenderSessionHelper, sync tools) open the clipboard after every change;
// holds of 20-75 ms were measured on 2026-11-19, back to back across
// listeners. The budget is a wall-clock deadline, so it does not depend on
// the Sleep granularity (1 ms or 15.6 ms timer resolution; the former
// 20 x Sleep(5) loop lasted anywhere from 100 to 310 ms). Backoff doubles
// from 2 ms up to 32 ms and the last sleep is clamped to the deadline, so a
// failing call returns after at most ~500 ms plus one OpenClipboard.
bool open_win32_clipboard() {
  constexpr ULONGLONG budget_ms = 500;
  constexpr DWORD max_delay_ms = 32;
  const ULONGLONG deadline = GetTickCount64() + budget_ms;
  DWORD delay_ms = 2;
  for (;;) {
    if (OpenClipboard(nullptr) != FALSE) return true;
    const ULONGLONG now = GetTickCount64();
    if (now >= deadline) return false;
    Sleep(static_cast<DWORD>(
        (std::min)(ULONGLONG{delay_ms}, deadline - now)));
    delay_ms = (std::min)(delay_ms * 2, max_delay_ms);
  }
}

} // namespace cgpui
#endif
