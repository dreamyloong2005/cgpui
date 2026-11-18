#pragma once

#include "cgpui/core/geometry.hpp"

namespace cgpui {

// GPUI `anchored()` placement: the overlay corner named by `anchor` sits on
// `position` (window coordinates).
enum class AnchorCorner {
  top_left,
  top_right,
  bottom_left,
  bottom_right,
};

// switch_anchor flips the corner on each axis that overflows the window and
// then shifts what still overflows; snap_to_window only shifts. Both keep
// `margin` logical pixels from the window edges.
enum class AnchoredFitMode {
  switch_anchor,
  snap_to_window,
};

struct AnchoredPlacement {
  Point position;
  AnchorCorner anchor = AnchorCorner::top_left;
  AnchoredFitMode fit_mode = AnchoredFitMode::switch_anchor;
  float margin = 4.0F;
};

// Side of an anchor rectangle the overlay opens on (tooltips below an
// element, submenus to the right of their item).
enum class AnchoredSide {
  below,
  above,
  right,
  left,
};

struct AnchoredBounds {
  Rect bounds;
  bool flipped_x = false;
  bool flipped_y = false;
  bool shifted = false;
};

[[nodiscard]] AnchoredBounds anchored_bounds(
    const AnchoredPlacement& placement,
    Size content,
    Size window);

// Opens on `side` of `anchor` with `gap`; flips to the opposite side when
// the preferred side overflows and the opposite side has more room, then
// shifts along the edge to stay inside the window. Below / above align the
// overlay's left edge with the anchor; right / left align the top edges.
[[nodiscard]] AnchoredBounds anchored_bounds_beside(
    Rect anchor,
    AnchoredSide side,
    Size content,
    Size window,
    float gap = 0.0F,
    float margin = 4.0F);

} // namespace cgpui
