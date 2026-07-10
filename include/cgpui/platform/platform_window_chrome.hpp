#pragma once

#include "cgpui/core/window.hpp"

#include <string>

namespace cgpui {

struct PlatformWindowChromeState {
  bool supported = false;
  bool decoration_control_supported = false;
  bool transparency_supported = false;
  std::string backend;
  WindowChromeOptions requested;
  WindowChromeOptions applied;
  std::string reason;
};

} // namespace cgpui
