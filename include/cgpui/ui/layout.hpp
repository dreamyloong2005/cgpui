#pragma once

#include "cgpui/core/geometry.hpp"

#include <algorithm>
#include <limits>

namespace cgpui {

struct LayoutConstraints {
  Size min_size;
  Size max_size{
      .width = std::numeric_limits<float>::infinity(),
      .height = std::numeric_limits<float>::infinity(),
  };
};

struct LayoutInput {
  LayoutConstraints constraints;
};

struct LayoutOutput {
  Point origin;
  Size size;
};

[[nodiscard]] inline Size constrain_size(
    Size preferred,
    LayoutConstraints constraints) {
  return Size{
      .width =
          std::clamp(preferred.width,
                     constraints.min_size.width,
                     constraints.max_size.width),
      .height =
          std::clamp(preferred.height,
                     constraints.min_size.height,
                     constraints.max_size.height),
  };
}

} // namespace cgpui
