#pragma once

#include "cgpui/ui/style_box.hpp"
#include "cgpui/ui/style_overlay.hpp"
#include "cgpui/ui/style_tokens.hpp"

namespace cgpui {

[[nodiscard]] Style resolve_style_theme_tokens(
    Style style,
    const Theme& theme);
[[nodiscard]] StyleOverlay resolve_overlay_theme_tokens(
    StyleOverlay overlay,
    const Theme& theme);

} // namespace cgpui
