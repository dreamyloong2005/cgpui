#include "paint_clip.hpp"

#include <algorithm>

namespace cgpui {

Rect intersect_clip_rect(Rect first, Rect second) {
  const float left = std::max(first.origin.x, second.origin.x);
  const float top = std::max(first.origin.y, second.origin.y);
  const float right = std::min(
      first.origin.x + first.size.width,
      second.origin.x + second.size.width);
  const float bottom = std::min(
      first.origin.y + first.size.height,
      second.origin.y + second.size.height);

  if (right <= left || bottom <= top) {
    return Rect{
        .origin = second.origin,
        .size = {.width = 0.0F, .height = 0.0F},
    };
  }

  return Rect{
      .origin = {.x = left, .y = top},
      .size =
          {
              .width = std::max(0.0F, right - left),
              .height = std::max(0.0F, bottom - top),
          },
  };
}

Rect effective_nested_clip_rect(
    const std::vector<Rect>& active_clips,
    Rect next_clip) {
  if (active_clips.empty()) {
    return next_clip;
  }
  return intersect_clip_rect(active_clips.back(), next_clip);
}

} // namespace cgpui
