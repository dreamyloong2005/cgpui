#include "win32_dead_key_internal.hpp"

namespace cgpui {

std::optional<Win32DeadKeyMessage> decode_win32_dead_key(
    UINT message,
    WPARAM wparam) {
  if (message != WM_DEADCHAR && message != WM_SYSDEADCHAR) {
    return std::nullopt;
  }
  return Win32DeadKeyMessage{
      .character = static_cast<wchar_t>(wparam),
      .system = message == WM_SYSDEADCHAR,
  };
}

} // namespace cgpui
