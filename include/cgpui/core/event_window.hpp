#pragma once

#include "cgpui/core/geometry.hpp"

namespace cgpui {

struct WindowCloseRequested {};

struct WindowRedrawRequested {};

struct WindowWakeupRequested {};

struct WindowResized {
  Size size;
  DpiScale scale;
};

struct WindowActivated {
  bool active = false;
};

struct WindowFocused {
  bool focused = false;
};

struct WindowMinimized {
  bool minimized = false;
};

struct WindowRestored {};

} // namespace cgpui
