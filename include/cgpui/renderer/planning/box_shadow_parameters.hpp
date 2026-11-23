#pragma once

namespace cgpui {

// CSS box-shadow blur radius to Gaussian standard deviation (blur / 2).
[[nodiscard]] float box_shadow_sigma(float blur_radius);
// CSS spread applied to one corner radius (css-backgrounds-3 7.1.1).
[[nodiscard]] float box_shadow_spread_radius(float radius, float spread);

} // namespace cgpui
