#include "clipboard_win32_internal.hpp"

#if defined(_WIN32)
namespace cgpui {

bool open_win32_clipboard() {
  constexpr int attempts = 20;
  constexpr DWORD retry_delay_ms = 5;
  for (int attempt = 0; attempt < attempts; ++attempt) {
    if (OpenClipboard(nullptr) != FALSE) return true;
    Sleep(retry_delay_ms);
  }
  return false;
}

} // namespace cgpui
#endif
