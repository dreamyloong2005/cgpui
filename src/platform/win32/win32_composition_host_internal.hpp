#pragma once

namespace cgpui {

// Platform hosting supports targets/visuals, independently of any renderer's
// device selection or interop bridge. No public window-mode choice is needed.
[[nodiscard]] bool win32_composition_host_available();

} // namespace cgpui
