#pragma once

#include "cgpui/core/window_title_bar.hpp"
#include "cgpui/platform/platform_window.hpp"

#include <vector>

namespace cgpui {

// Sends this frame's client title bar regions to `window`. `regions` holds
// what the frame painted: View::paint's regions, then the element tree's
// (render_view paints the tree after View::paint).
void runtime_publish_window_control_regions(
    PlatformWindow* window,
    std::vector<WindowControlRegion> regions);

} // namespace cgpui
