#pragma once

#include "cgpui/platform/platform_dpi_awareness.hpp"

namespace cgpui {

// Wayland has no process-wide awareness switch. CGPUI scales each surface
// itself: wp_fractional_scale_v1 + wp_viewporter give a per-surface
// fractional scale (reported as per_monitor_aware_v2); without them the
// integer wl_output scale becomes the wl_surface buffer scale
// (per_monitor_aware).
[[nodiscard]] PlatformDpiAwarenessState wayland_dpi_awareness_state(
    bool fractional_scale_available);

} // namespace cgpui
