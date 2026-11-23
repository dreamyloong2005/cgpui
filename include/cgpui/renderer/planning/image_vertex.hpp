#pragma once

#include <array>
#include <cstddef>

namespace cgpui {

struct ImageVertex {
  std::array<float, 2> position{};
  std::array<float, 2> image_uv{};
  std::array<float, 4> color{};
};

inline constexpr std::size_t image_vertices_per_quad = 6;

} // namespace cgpui
