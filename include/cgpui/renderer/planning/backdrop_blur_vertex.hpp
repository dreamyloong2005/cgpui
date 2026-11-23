#pragma once

#include <array>

namespace cgpui {

// Composite quad vertex. `region` = {scene origin x, scene origin y,
// downsample factor, composed opacity}; rect/radii are in local space.
struct BackdropBlurVertex {
  std::array<float, 2> position{};
  std::array<float, 2> local{};
  std::array<float, 4> rect{};
  std::array<float, 4> radii{};
  std::array<float, 4> tint{};
  std::array<float, 4> region{};
};

} // namespace cgpui
