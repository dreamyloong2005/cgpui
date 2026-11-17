#pragma once

#include "cgpui/core/window_title_bar.hpp"
#include "cgpui/platform/platform_window.hpp"
#include "cgpui/ui/element_tree.hpp"

#include <vector>

namespace cgpui {

// Sends this frame's client title bar regions to `window`. `regions` holds
// what View::paint declared. The runtime frame does not paint element trees
// into the renderer, so while the window is in client title bar mode the
// laid-out `tree` gets one extra paint pass into a scratch PaintList, used
// only to collect the regions its elements declare.
void runtime_publish_window_control_regions(
    PlatformWindow* window,
    const ElementTree* tree,
    DpiScale scale,
    std::vector<WindowControlRegion> regions);

} // namespace cgpui
