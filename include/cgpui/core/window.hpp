#pragma once

#include "cgpui/core/geometry.hpp"

#include <string>

namespace cgpui {

struct WindowDescriptor {
  std::string title = "CGPUI";
  Size size{1280.0F, 720.0F};
};

struct WindowState {
  Size framebuffer_size;
  DpiScale scale;
  bool close_requested = false;
};

} // namespace cgpui
