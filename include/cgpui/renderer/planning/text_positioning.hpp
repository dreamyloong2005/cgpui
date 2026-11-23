#pragma once

#include "cgpui/core/geometry.hpp"

namespace cgpui {

enum class TextPositioningPolicy {
  preserve_subpixel,
  snap_to_device_pixels,
};

[[nodiscard]] TextPositioningPolicy default_text_positioning_policy();
[[nodiscard]] Rect position_text_bounds(Rect bounds, TextPositioningPolicy policy);

} // namespace cgpui
