#pragma once

namespace cgpui {

enum class RectAntialiasingMode {
  coverage_fringe,
};

struct RectAntialiasingPolicy {
  RectAntialiasingMode mode =
      RectAntialiasingMode::coverage_fringe;
  float fringe_width = 1.0F;
};

[[nodiscard]] RectAntialiasingPolicy
default_rounded_rect_antialiasing_policy();
[[nodiscard]] float rounded_rect_coverage(float coverage);

} // namespace cgpui
