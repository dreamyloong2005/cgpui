#pragma once

#include "cgpui/platform/platform_dpi_awareness.hpp"

namespace cgpui {

// Declares Per-Monitor V2 awareness for the process once. Later calls return
// the cached result. A host manifest or earlier API call that already fixed
// the process awareness is reported, not treated as an error.
PlatformDpiAwarenessState win32_declare_dpi_awareness();

[[nodiscard]] PlatformDpiAwarenessState win32_current_dpi_awareness();

} // namespace cgpui
