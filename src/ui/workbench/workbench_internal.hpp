#pragma once

#include "cgpui/core/geometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace cgpui::workbench_detail {

// Virtual key codes delivered in KeyboardKey::key_code on every backend.
inline constexpr std::uint32_t key_tab = 9;
inline constexpr std::uint32_t key_enter = 13;
inline constexpr std::uint32_t key_escape = 27;
inline constexpr std::uint32_t key_space = 32;
inline constexpr std::uint32_t key_page_up = 33;
inline constexpr std::uint32_t key_page_down = 34;
inline constexpr std::uint32_t key_end = 35;
inline constexpr std::uint32_t key_home = 36;
inline constexpr std::uint32_t key_left = 37;
inline constexpr std::uint32_t key_up = 38;
inline constexpr std::uint32_t key_right = 39;
inline constexpr std::uint32_t key_down = 40;
inline constexpr std::uint32_t key_delete = 46;
inline constexpr std::uint32_t key_apps = 93;
inline constexpr std::uint32_t key_f10 = 121;

[[nodiscard]] inline float scale_of(DpiScale scale) {
  return scale.value > 0.0F ? scale.value : 1.0F;
}

// Rounds a logical coordinate to the nearest device pixel.
[[nodiscard]] inline float snap(float value, DpiScale scale) {
  const float factor = scale_of(scale);
  return std::round(value * factor) / factor;
}

// A logical length of at least one device pixel.
[[nodiscard]] inline float snap_length(float value, DpiScale scale) {
  const float factor = scale_of(scale);
  return std::max(1.0F, std::round(value * factor)) / factor;
}

[[nodiscard]] inline float right(Rect rect) {
  return rect.origin.x + rect.size.width;
}

[[nodiscard]] inline float bottom(Rect rect) {
  return rect.origin.y + rect.size.height;
}

} // namespace cgpui::workbench_detail
