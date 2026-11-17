#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/core/window_title_bar.hpp"

#include <string>

namespace cgpui {

struct WindowFrameInsets {
  float top = 0.0F;
  float right = 0.0F;
  float bottom = 0.0F;
  float left = 0.0F;
};

// Native frame metrics for laying out a client-drawn title bar. All sizes are
// logical pixels at the window's current scale.
struct PlatformWindowFrameMetrics {
  bool supported = false;
  bool client_title_bar = false;  // client mode is active on this window
  std::string backend;
  float caption_height = 0.0F;  // system title bar height; 0 = no suggestion
  Size caption_button_size;     // system caption button size; 0 = none
  float resize_border = 0.0F;   // resize band thickness used for hit tests
  // Parts of the resize band that lie inside the client area (the app should
  // keep interactive content out of them). Zero when maximized.
  WindowFrameInsets client_resize_border;
  // Width at the right of the caption row still used by native buttons.
  float caption_buttons_reserved_width = 0.0F;
  std::string mechanism;
  std::string reason;
};

enum class PlatformWindowAction {
  minimize,
  toggle_maximize,
  close,
  show_window_menu,  // `position` is a client point in logical pixels
  begin_move,
  begin_resize,  // uses `edge`
};

struct PlatformWindowActionRequest {
  PlatformWindowAction action = PlatformWindowAction::minimize;
  Point position;
  WindowResizeEdge edge = WindowResizeEdge::bottom_right;
};

struct PlatformWindowActionResult {
  bool supported = false;
  bool performed = false;
  std::string reason;
};

} // namespace cgpui
