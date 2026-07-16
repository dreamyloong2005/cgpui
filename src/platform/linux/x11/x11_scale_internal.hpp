#pragma once

#include "cgpui/core/geometry.hpp"

#include <xcb/xcb.h>

#include <optional>
#include <string_view>

namespace cgpui {

[[nodiscard]] std::optional<float> x11_parse_xft_dpi(std::string_view resources);
[[nodiscard]] DpiScale x11_display_scale(
    xcb_connection_t* connection,
    xcb_screen_t* screen);

}  // namespace cgpui
