#pragma once

#include "cgpui/core/window.hpp"
#include "cgpui/core/window_popup.hpp"

#import <AppKit/AppKit.h>

namespace cgpui {

// Placement math: platform_popup_frame (platform_popup_placement_internal.hpp);
// macos_popup_window.mm converts AppKit's y-up screen coordinates for it.

// A frameless, non-activating, transparent NSPanel at the pop-up menu
// level that hides when the app deactivates.
[[nodiscard]] NSPanel* macos_make_popup_panel(const WindowDescriptor& descriptor);
// Adds the panel as a child of `parent`, then applies the popup level and
// hides-on-deactivate (adding a child window resets both).
void macos_attach_popup_panel(NSPanel* panel, NSWindow* parent);

} // namespace cgpui
