#pragma once

#include "cgpui/core/event_keyboard.hpp"
#include "cgpui/platform/target.hpp"

#include <string>
#include <string_view>

namespace cgpui {

[[nodiscard]] std::string lower_key_binding_token(std::string_view value);

[[nodiscard]] bool apply_key_binding_modifier_token(
    std::string_view token,
    DesktopPlatformTarget platform,
    KeyboardModifiers& modifiers);

} // namespace cgpui
