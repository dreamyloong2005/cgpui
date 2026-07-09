#pragma once

#include "cgpui/core/geometry.hpp"

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

  void set_offset(Point offset);
  void scroll_by(Point delta);
  void scroll_rect_into_view(Rect rect);

  void set_viewport_size(Size size);

  void set_content_size(Size size);

 private:
  [[nodiscard]] Point max_offset() const;
  [[nodiscard]] Point clamp_offset(Point offset) const;
  [[nodiscard]] static Size clamp_non_negative(Size size);

  Point offset_;
  Size viewport_size_;
  Size content_size_;
};

using ScrollState = ScrollModel;

} // namespace cgpui
