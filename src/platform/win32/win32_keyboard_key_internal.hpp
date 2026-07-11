#pragma once

#include "win32_internal.hpp"

#include <optional>

namespace cgpui {

[[nodiscard]] std::optional<KeyboardKey> decode_win32_keyboard_key(
    UINT message,
    WPARAM wparam,
    LPARAM lparam);

} // namespace cgpui
