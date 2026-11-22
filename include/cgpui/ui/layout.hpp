#pragma once

#include "cgpui/core/pixel_scale.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace cgpui {

class ElementAnimationStateStore;

struct LayoutConstraints {
  Size min_size;
  Size max_size{
      .width = std::numeric_limits<float>::infinity(),
      .height = std::numeric_limits<float>::infinity(),
  };
};

struct LayoutInput {
  LayoutConstraints constraints;
  DpiScale scale;
  ElementAnimationStateStore* animation_state_store = nullptr;
  std::uint64_t animation_scope_id = 0;
  std::uint64_t animation_time_ms = 0;
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
