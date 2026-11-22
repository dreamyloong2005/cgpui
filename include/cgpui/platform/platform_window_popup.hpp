#pragma once

#include "cgpui/core/window_popup.hpp"

#include <string>

namespace cgpui {

// Result of placing a popup window. `bounds` is the popup's frame in the
// parent's logical client coordinates after flipping and shifting.
struct PlatformPopupPlacementResult {
  bool supported = false;
  bool placed = false;
  Rect bounds;
  bool flipped = false;
  bool shifted = false;
  std::string reason;
};

} // namespace cgpui
