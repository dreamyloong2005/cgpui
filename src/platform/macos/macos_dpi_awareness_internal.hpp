#pragma once

#include "cgpui/platform/platform_dpi_awareness.hpp"

namespace cgpui {

// AppKit has no process-wide awareness switch. CGPUI scales each window
// itself from NSWindow.backingScaleFactor and follows
// windowDidChangeBackingProperties: when a window moves to a display with
// another scale (reported as per_monitor_aware_v2, as on Wayland with
// fractional scaling).
[[nodiscard]] PlatformDpiAwarenessState macos_dpi_awareness_state();

} // namespace cgpui
