#pragma once

#include "cgpui/core/window_backdrop.hpp"

#include <string>

namespace cgpui {

// Result of applying or querying a window backdrop. `applied` is the
// material visible behind the client area; it is `none` whenever the backend
// cannot show the request there, and `reason` then says why.
struct PlatformWindowBackdropState {
  // The backend has a mechanism for the requested material on this window.
  bool supported = false;
  // The client surface composites with alpha, so a material can show
  // through transparent pixels.
  bool transparent_surface = false;
  std::string backend;
  WindowBackdrop requested = WindowBackdrop::none;
  WindowBackdrop applied = WindowBackdrop::none;
  // Material on the system-drawn title bar (Windows only). It can be set
  // while `applied` is none, for example when the surface is opaque.
  WindowBackdrop title_bar = WindowBackdrop::none;
  // Native mechanism in use, for example "DWMWA_SYSTEMBACKDROP_TYPE=2".
  std::string mechanism;
  std::string reason;
};

} // namespace cgpui
