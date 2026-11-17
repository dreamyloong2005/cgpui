#pragma once

#include "cgpui/core/geometry.hpp"

namespace cgpui {

// Who draws the title bar row. `client` keeps the native frame behaviors
// (move, resize, snap, system menu) but lets the app draw the row itself.
enum class WindowTitleBarMode { system, client };

// What a painted region of a client-drawn title bar means to the platform.
// Mirrors GPUI's WindowControlArea (Drag, Min, Max, Close); `client` marks an
// interactive control inside the caption row that must not drag.
enum class WindowControlArea {
  caption,
  minimize_button,
  maximize_button,
  close_button,
  client,
};

// `bounds` is in logical pixels, relative to the client area origin.
struct WindowControlRegion {
  WindowControlArea area = WindowControlArea::caption;
  Rect bounds;
};

enum class WindowResizeEdge {
  top,
  bottom,
  left,
  right,
  top_left,
  top_right,
  bottom_left,
  bottom_right,
};

} // namespace cgpui
