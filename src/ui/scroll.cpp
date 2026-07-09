#include "cgpui/ui/scroll.hpp"

#include <algorithm>

namespace cgpui {

void ScrollModel::set_offset(Point offset) {
  offset_ = clamp_offset(offset);
}

void ScrollModel::scroll_by(Point delta) {
  set_offset(Point{
      .x = offset_.x + delta.x,
      .y = offset_.y + delta.y,
  });
}

void ScrollModel::scroll_rect_into_view(Rect rect) {
  const Size rect_size = clamp_non_negative(rect.size);
  const float rect_right = rect.origin.x + rect_size.width;
  const float rect_bottom = rect.origin.y + rect_size.height;
  Point target = offset_;

  if (rect.origin.x < target.x) {
    target.x = rect.origin.x;
  } else if (rect_right > target.x + viewport_size_.width) {
    target.x = rect_right - viewport_size_.width;
  }

  if (rect.origin.y < target.y) {
    target.y = rect.origin.y;
  } else if (rect_bottom > target.y + viewport_size_.height) {
    target.y = rect_bottom - viewport_size_.height;
  }

  set_offset(target);
}

void ScrollModel::set_viewport_size(Size size) {
  viewport_size_ = clamp_non_negative(size);
  offset_ = clamp_offset(offset_);
}

void ScrollModel::set_content_size(Size size) {
  content_size_ = clamp_non_negative(size);
  offset_ = clamp_offset(offset_);
}

Point ScrollModel::max_offset() const {
  return Point{
      .x = std::max(0.0F, content_size_.width - viewport_size_.width),
      .y = std::max(0.0F, content_size_.height - viewport_size_.height),
  };
}

Point ScrollModel::clamp_offset(Point offset) const {
  const Point max = max_offset();
  return Point{
      .x = std::clamp(offset.x, 0.0F, max.x),
      .y = std::clamp(offset.y, 0.0F, max.y),
  };
}

Size ScrollModel::clamp_non_negative(Size size) {
  return Size{
      .width = std::max(0.0F, size.width),
      .height = std::max(0.0F, size.height),
  };
}

} // namespace cgpui
