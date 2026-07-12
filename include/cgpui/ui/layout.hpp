#pragma once

#include "cgpui/core/geometry.hpp"

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

[[nodiscard]] inline float normalized_scale(DpiScale scale) {
  return scale.value > 0.0F ? scale.value : 1.0F;
}

[[nodiscard]] inline Size to_logical_pixels(Size device_size, DpiScale scale) {
  const float value = normalized_scale(scale);
  return Size{
      .width = device_size.width / value,
      .height = device_size.height / value,
  };
}

[[nodiscard]] inline Point to_logical_pixels(Point device_point, DpiScale scale) {
  const float value = normalized_scale(scale);
  return Point{
      .x = device_point.x / value,
      .y = device_point.y / value,
  };
}

[[nodiscard]] inline Rect to_logical_pixels(Rect device_rect, DpiScale scale) {
  return Rect{
      .origin = to_logical_pixels(device_rect.origin, scale),
      .size = to_logical_pixels(device_rect.size, scale),
  };
}

[[nodiscard]] inline Size to_device_pixels(Size logical_size, DpiScale scale) {
  const float value = normalized_scale(scale);
  return Size{
      .width = logical_size.width * value,
      .height = logical_size.height * value,
  };
}

[[nodiscard]] inline Point to_device_pixels(Point logical_point, DpiScale scale) {
  const float value = normalized_scale(scale);
  return Point{
      .x = logical_point.x * value,
      .y = logical_point.y * value,
  };
}

[[nodiscard]] inline Rect to_device_pixels(Rect logical_rect, DpiScale scale) {
  return Rect{
      .origin = to_device_pixels(logical_rect.origin, scale),
      .size = to_device_pixels(logical_rect.size, scale),
  };
}

} // namespace cgpui
