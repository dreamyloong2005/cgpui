#include "cgpui/renderer/planning/rect_antialiasing.hpp"

#include <algorithm>

namespace cgpui {

RectAntialiasingPolicy
default_rounded_rect_antialiasing_policy() {
  return {};
}

float rounded_rect_coverage(float coverage) {
  return std::clamp(coverage, 0.0F, 1.0F);
}

} // namespace cgpui
