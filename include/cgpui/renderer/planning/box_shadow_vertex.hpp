#pragma once

#include <array>

namespace cgpui {

// One shadow is one quad. `position` is the transformed device position;
// the fragment shader evaluates the shadow at the untransformed `local`
// point against the shadow and box shapes, so an affine transform scales
// and rotates the shadow with its box. Rects are {min x, min y, max x,
// max y}; radii are {top left, top right, bottom right, bottom left}.
struct BoxShadowVertex {
  std::array<float, 2> position{};
  std::array<float, 2> local{};
  std::array<float, 4> color{};
  std::array<float, 4> shadow_rect{};
  std::array<float, 4> shadow_radii{};
  std::array<float, 4> box_rect{};
  std::array<float, 4> box_radii{};
  float sigma = 0.0F;
};

} // namespace cgpui
