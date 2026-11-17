#include "cgpui/ui/paint.hpp"

#include "paint_clip.hpp"
#include "paint_clip_transform.hpp"

namespace cgpui {

void PaintList::add_window_control_region(WindowControlArea area, Rect bounds) {
  const AffineTransform transform = metadata_stack_.empty()
      ? AffineTransform::identity()
      : metadata_stack_.back().transform;
  Rect visible = transform_clip_rect_to_framebuffer_aabb(bounds, transform);
  if (!clip_stack_.empty()) {
    visible = intersect_clip_rect(clip_stack_.back(), visible);
  }
  if (visible.size.width <= 0.0F || visible.size.height <= 0.0F) return;
  window_control_regions_.push_back(
      WindowControlRegion{.area = area, .bounds = visible});
}

std::span<const WindowControlRegion> PaintList::window_control_regions() const {
  return window_control_regions_;
}

} // namespace cgpui
