#pragma once

#include "cgpui/core/window_popup.hpp"

namespace cgpui {

struct PlatformPopupFrame {
  Rect bounds;
  bool flipped = false;
  bool shifted = false;
};

// Popup bounds of `size` on `side` of `anchor` with `gap`, in a y-down
// space shared by the anchor and the work area (backends convert screen
// coordinates). Below / above align left edges, right / left align top
// edges; the popup flips to the other side when the preferred side leaves
// `work_area` and the other side has more room, then shifts along each
// axis to stay on it (to its top / left edge when it is larger).
[[nodiscard]] PlatformPopupFrame platform_popup_frame(
    Rect anchor,
    Size size,
    WindowPopupSide side,
    float gap,
    Rect work_area);

} // namespace cgpui
