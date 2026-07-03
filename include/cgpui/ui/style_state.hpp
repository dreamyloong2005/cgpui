#pragma once

#include "cgpui/ui/style_box.hpp"
#include "cgpui/ui/style_overlay.hpp"

namespace cgpui {

struct StyleState {
  Style base;
  StyleOverlay hover;
  StyleOverlay focus;
  StyleOverlay disabled;
};

struct StyleStateFlags {
  bool hovered = false;
  bool focused = false;
  bool disabled = false;
};

} // namespace cgpui
