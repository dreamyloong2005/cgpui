#pragma once

#include "cgpui/core/geometry.hpp"

#include <algorithm>

namespace cgpui {

class ScrollModel {
 public:
  [[nodiscard]] Point offset() const {
    return offset_;
  }

  [[nodiscard]] Size viewport_size() const {
    return viewport_size_;
  }

  [[nodiscard]] Size content_size() const {
    return content_size_;
  }

  [[nodiscard]] bool can_scroll_x() const {
    return max_offset().x > 0.0F;
  }

  [[nodiscard]] bool can_scroll_y() const {
    return max_offset().y > 0.0F;
  }

  void set_offset(Point offset) {
    offset_ = clamp_offset(offset);
  }

  void scroll_by(Point delta) {
    set_offset(Point{
        .x = offset_.x + delta.x,
        .y = offset_.y + delta.y,
    });
  }

  void set_viewport_size(Size size) {
    viewport_size_ = clamp_non_negative(size);
    offset_ = clamp_offset(offset_);
  }

  void set_content_size(Size size) {
    content_size_ = clamp_non_negative(size);
    offset_ = clamp_offset(offset_);
  }

 private:
  [[nodiscard]] Point max_offset() const {
    return Point{
        .x = std::max(0.0F, content_size_.width - viewport_size_.width),
        .y = std::max(0.0F, content_size_.height - viewport_size_.height),
    };
  }

  [[nodiscard]] Point clamp_offset(Point offset) const {
    const Point max = max_offset();
    return Point{
        .x = std::clamp(offset.x, 0.0F, max.x),
        .y = std::clamp(offset.y, 0.0F, max.y),
    };
  }

  [[nodiscard]] static Size clamp_non_negative(Size size) {
    return Size{
        .width = std::max(0.0F, size.width),
        .height = std::max(0.0F, size.height),
    };
  }

  Point offset_;
  Size viewport_size_;
  Size content_size_;
};

using ScrollState = ScrollModel;

} // namespace cgpui
