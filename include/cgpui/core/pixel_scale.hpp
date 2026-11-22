#pragma once

#include "cgpui/core/geometry.hpp"

namespace cgpui {

[[nodiscard]] constexpr float normalized_scale(DpiScale scale) {
  return scale.value > 0.0F ? scale.value : 1.0F;
}

[[nodiscard]] constexpr Size to_logical_pixels(Size device_size, DpiScale scale) {
  const float value = normalized_scale(scale);
  return Size{
      .width = device_size.width / value,
      .height = device_size.height / value,
  };
}

[[nodiscard]] constexpr Point to_logical_pixels(Point device_point, DpiScale scale) {
  const float value = normalized_scale(scale);
  return Point{
      .x = device_point.x / value,
      .y = device_point.y / value,
  };
}

[[nodiscard]] constexpr Rect to_logical_pixels(Rect device_rect, DpiScale scale) {
  return Rect{
      .origin = to_logical_pixels(device_rect.origin, scale),
      .size = to_logical_pixels(device_rect.size, scale),
  };
}

[[nodiscard]] constexpr Size to_device_pixels(Size logical_size, DpiScale scale) {
  const float value = normalized_scale(scale);
  return Size{
      .width = logical_size.width * value,
      .height = logical_size.height * value,
  };
}

[[nodiscard]] constexpr Point to_device_pixels(Point logical_point, DpiScale scale) {
  const float value = normalized_scale(scale);
  return Point{
      .x = logical_point.x * value,
      .y = logical_point.y * value,
  };
}

[[nodiscard]] constexpr Rect to_device_pixels(Rect logical_rect, DpiScale scale) {
  return Rect{
      .origin = to_device_pixels(logical_rect.origin, scale),
      .size = to_device_pixels(logical_rect.size, scale),
  };
}

} // namespace cgpui
