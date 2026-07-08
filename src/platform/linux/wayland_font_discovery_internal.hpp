#pragma once

#include "cgpui/platform/platform_font_discovery.hpp"

namespace cgpui {

[[nodiscard]] PlatformFontDiscoveryResult wayland_discover_fonts_with_fontconfig();
[[nodiscard]] PlatformFontDiscoveryResult wayland_deterministic_font_fallback();

} // namespace cgpui
