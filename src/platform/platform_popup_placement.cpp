#include "platform_popup_placement_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

// Start of an overlay of `length` on one axis: `preferred` unless it
// leaves [low, high]; clamps to `low` when the overlay is longer.
float shift_into(float preferred, float length, float low, float high, bool& shifted) {
  const float value = std::max(low, std::min(preferred, high - length));
  shifted = shifted || value != preferred;
  return value;
}

} // namespace

PlatformPopupFrame platform_popup_frame(
    Rect anchor, Size size, WindowPopupSide side, float gap, Rect work) {
  const float left = anchor.origin.x;
  const float top = anchor.origin.y;
  const float right = left + anchor.size.width;
  const float bottom = top + anchor.size.height;
  const float work_right = work.origin.x + work.size.width;
  const float work_bottom = work.origin.y + work.size.height;
  PlatformPopupFrame frame{.bounds = {{left, top}, size}};
  const bool vertical = side == WindowPopupSide::below || side == WindowPopupSide::above;
  if (vertical) {
    const float room_below = work_bottom - bottom - gap;
    const float room_above = top - work.origin.y - gap;
    bool below = side == WindowPopupSide::below;
    if ((below && size.height > room_below && room_above > room_below) ||
        (!below && size.height > room_above && room_below > room_above)) {
      below = !below;
      frame.flipped = true;
    }
    frame.bounds.origin.y = below ? bottom + gap : top - gap - size.height;
  } else {
    const float room_right = work_right - right - gap;
    const float room_left = left - work.origin.x - gap;
    bool to_right = side == WindowPopupSide::right;
    if ((to_right && size.width > room_right && room_left > room_right) ||
        (!to_right && size.width > room_left && room_right > room_left)) {
      to_right = !to_right;
      frame.flipped = true;
    }
    frame.bounds.origin.x = to_right ? right + gap : left - gap - size.width;
  }
  frame.bounds.origin.x = shift_into(frame.bounds.origin.x, size.width, work.origin.x,
                                     work_right, frame.shifted);
  frame.bounds.origin.y = shift_into(frame.bounds.origin.y, size.height, work.origin.y,
                                     work_bottom, frame.shifted);
  return frame;
}

} // namespace cgpui
