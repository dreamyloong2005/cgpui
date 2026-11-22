#pragma once

#include "cgpui/core/window.hpp"
#include "cgpui/core/window_popup.hpp"

#import <AppKit/AppKit.h>

namespace cgpui {

struct MacOSPopupFrame {
  Rect bounds;
  bool flipped = false;
  bool shifted = false;
};

// Popup frame for `placement` in a y-down space shared by the anchor and
// the work area (macos_popup_window.mm converts screen coordinates).
[[nodiscard]] MacOSPopupFrame macos_popup_frame(
    Rect anchor,
    Size size,
    WindowPopupSide side,
    float gap,
    Rect work_area);

// A frameless, non-activating, transparent NSPanel at the pop-up menu
// level that hides when the app deactivates.
[[nodiscard]] NSPanel* macos_make_popup_panel(const WindowDescriptor& descriptor);
// Adds the panel as a child of `parent`, then applies the popup level and
// hides-on-deactivate (adding a child window resets both).
void macos_attach_popup_panel(NSPanel* panel, NSWindow* parent);

} // namespace cgpui
