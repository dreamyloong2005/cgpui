#pragma once

#include "win32_internal.hpp"

#include <optional>

namespace cgpui {

struct Win32PointerScrollMessage {
  Point delta;
  bool precise = false;
};

[[nodiscard]] std::optional<Win32PointerScrollMessage>
decode_win32_pointer_scroll(UINT message, WPARAM wparam);

} // namespace cgpui
