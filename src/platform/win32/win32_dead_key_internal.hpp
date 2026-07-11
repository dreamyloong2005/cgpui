#pragma once

#include "win32_internal.hpp"

#include <optional>

namespace cgpui {

struct Win32DeadKeyMessage {
  wchar_t character = 0;
  bool system = false;
};

[[nodiscard]] std::optional<Win32DeadKeyMessage> decode_win32_dead_key(
    UINT message,
    WPARAM wparam);

} // namespace cgpui
