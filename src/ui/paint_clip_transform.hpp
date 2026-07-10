#pragma once

#include "cgpui/core/geometry.hpp"

namespace cgpui {

[[nodiscard]] Rect transform_clip_rect_to_framebuffer_aabb(
    Rect clip,
    AffineTransform transform);

} // namespace cgpui
