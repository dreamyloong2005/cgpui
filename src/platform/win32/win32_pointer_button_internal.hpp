#pragma once

#include "win32_internal.hpp"

#include <optional>

namespace cgpui {

struct Win32PointerButtonMessage {
  MouseButton button = MouseButton::other;
  bool pressed = false;
  std::uint8_t click_count = 1;
  LRESULT result = 0;
};

[[nodiscard]] std::optional<Win32PointerButtonMessage>
decode_win32_pointer_button(UINT message, WPARAM wparam);

} // namespace cgpui
