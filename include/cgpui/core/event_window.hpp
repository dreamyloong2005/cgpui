#pragma once

#include "cgpui/core/geometry.hpp"

#include <cstdint>

namespace cgpui {

enum class WindowCloseRequestSource {
  window_manager,
  application,
};

struct WindowCloseRequested {
  WindowCloseRequestSource source =
      WindowCloseRequestSource::window_manager;
  std::uint64_t sequence = 0;
};

struct WindowRedrawRequested {};

struct WindowWakeupRequested {};

struct WindowResized {
  Size size;
  DpiScale scale;
};

struct WindowMoved {
  Point position;
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
