#pragma once

#include "cgpui/core/geometry.hpp"

#include <optional>

namespace cgpui {

struct PlatformWindowPositionState {
  bool supported = false;
  std::optional<Point> position;
};

} // namespace cgpui
