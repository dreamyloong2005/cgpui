#pragma once

#include "cgpui/core/geometry.hpp"

#include <vector>

namespace cgpui {

[[nodiscard]] Rect intersect_clip_rect(Rect first, Rect second);
[[nodiscard]] Rect effective_nested_clip_rect(
    const std::vector<Rect>& active_clips,
    Rect next_clip);

} // namespace cgpui
