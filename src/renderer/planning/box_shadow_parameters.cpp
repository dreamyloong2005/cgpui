#include "cgpui/renderer/planning/box_shadow_parameters.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {

float box_shadow_sigma(float blur_radius) {
  return std::isfinite(blur_radius) ? std::max(0.0F, blur_radius) * 0.5F
                                    : 0.0F;
}

float box_shadow_spread_radius(float radius, float spread) {
  if (radius <= 0.0F) {
    return 0.0F;
  }
  if (spread <= 0.0F || radius >= spread) {
    return std::max(0.0F, radius + spread);
  }
  const float ratio = radius / spread - 1.0F;
  return radius + spread * (1.0F + ratio * ratio * ratio);
}

} // namespace cgpui
