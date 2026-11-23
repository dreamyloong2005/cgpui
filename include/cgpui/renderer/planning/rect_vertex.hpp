#pragma once

#include <array>

namespace cgpui {

struct RectVertex {
  std::array<float, 2> position{};
  std::array<float, 4> color{};
  float coverage = 1.0F;
};

} // namespace cgpui
