#include "cgpui/ui/anchored.hpp"

#include <algorithm>

namespace cgpui {
namespace {

// Keeps [start, start + extent) inside [margin, window - margin]; an overlay
// larger than the window is pinned to the leading margin.
float shift_into(float start, float extent, float window, float margin) {
  const float high = window - margin - extent;
  if (start > high) start = high;
  if (start < margin) start = margin;
  return start;
}

bool left_anchor(AnchorCorner corner) {
  return corner == AnchorCorner::top_left || corner == AnchorCorner::bottom_left;
}

bool top_anchor(AnchorCorner corner) {
  return corner == AnchorCorner::top_left || corner == AnchorCorner::top_right;
}

} // namespace

AnchoredBounds anchored_bounds(
    const AnchoredPlacement& placement,
    Size content,
    Size window) {
  const float margin = placement.margin;
  bool left = left_anchor(placement.anchor);
  bool top = top_anchor(placement.anchor);
  AnchoredBounds result;
  const auto x_for = [&](bool from_left) {
    return from_left ? placement.position.x
                     : placement.position.x - content.width;
  };
  const auto y_for = [&](bool from_top) {
    return from_top ? placement.position.y
                    : placement.position.y - content.height;
  };
  const auto overflows = [&](float start, float extent, float limit) {
    return start < margin || start + extent > limit - margin;
  };
  if (placement.fit_mode == AnchoredFitMode::switch_anchor) {
    if (overflows(x_for(left), content.width, window.width) &&
        !overflows(x_for(!left), content.width, window.width)) {
      left = !left;
      result.flipped_x = true;
    }
    if (overflows(y_for(top), content.height, window.height) &&
        !overflows(y_for(!top), content.height, window.height)) {
      top = !top;
      result.flipped_y = true;
    }
  }
  const Point start{x_for(left), y_for(top)};
  const Point fitted{
      shift_into(start.x, content.width, window.width, margin),
      shift_into(start.y, content.height, window.height, margin)};
  result.shifted = fitted.x != start.x || fitted.y != start.y;
  result.bounds = Rect{.origin = fitted, .size = content};
  return result;
}

AnchoredBounds anchored_bounds_beside(
    Rect anchor,
    AnchoredSide side,
    Size content,
    Size window,
    float gap,
    float margin) {
  const bool vertical = side == AnchoredSide::below || side == AnchoredSide::above;
  const float anchor_start = vertical ? anchor.origin.y : anchor.origin.x;
  const float anchor_extent = vertical ? anchor.size.height : anchor.size.width;
  const float extent = vertical ? content.height : content.width;
  const float limit = vertical ? window.height : window.width;
  const bool after = side == AnchoredSide::below || side == AnchoredSide::right;
  const float after_start = anchor_start + anchor_extent + gap;
  const float before_start = anchor_start - gap - extent;
  const float after_room = limit - margin - after_start;
  const float before_room = before_start + extent - margin;
  bool use_after = after;
  if (after && extent > after_room && before_room > after_room) use_after = false;
  if (!after && extent > before_room && after_room > before_room) use_after = true;
  AnchoredBounds result;
  result.flipped_y = vertical && use_after != after;
  result.flipped_x = !vertical && use_after != after;
  float main = use_after ? after_start : before_start;
  const float fitted_main = shift_into(main, extent, limit, margin);
  result.shifted = fitted_main != main;
  main = fitted_main;
  const float cross_start = vertical ? anchor.origin.x : anchor.origin.y;
  const float cross_extent = vertical ? content.width : content.height;
  const float cross_limit = vertical ? window.width : window.height;
  const float cross = shift_into(cross_start, cross_extent, cross_limit, margin);
  result.shifted = result.shifted || cross != cross_start;
  result.bounds = Rect{
      .origin = vertical ? Point{cross, main} : Point{main, cross},
      .size = content,
  };
  return result;
}

} // namespace cgpui
