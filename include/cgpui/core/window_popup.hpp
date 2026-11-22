#pragma once

#include "cgpui/core/geometry.hpp"

namespace cgpui {

// `popup`: a frameless child window for tooltips, menus and popovers. It
// never takes key focus, stays out of the window list, follows its parent
// and may extend outside the parent's bounds. Popups need a parent window
// and backend support (PlatformApplication::popup_windows_supported).
enum class WindowKind { normal, popup };

enum class WindowPopupSide { below, above, right, left };

// Where a popup opens: on `side` of `anchor` (the parent's logical client
// coordinates) with `gap`. The backend flips to the opposite side when the
// preferred side leaves the screen's work area and the other side has more
// room, then shifts along the edge to stay on screen. Below / above align
// left edges; right / left align top edges (as anchored_bounds_beside).
struct WindowPopupPlacement {
  Rect anchor;
  WindowPopupSide side = WindowPopupSide::below;
  float gap = 0.0F;
};

struct WindowPopupOptions {
  // Pointer input passes through to what is below (tooltips), so the popup
  // never takes hover from the window that owns it.
  bool pointer_transparent = false;
  // The root window's key and text events go to the popup while it is
  // open (popups are never key windows).
  bool takes_keyboard = false;
};

} // namespace cgpui
