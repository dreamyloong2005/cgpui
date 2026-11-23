#pragma once

#include <array>
#include <cstddef>

namespace cgpui {

struct TextVertex {
  std::array<float, 2> position{};
  std::array<float, 2> atlas_uv{};
  std::array<float, 4> color{};
};

inline constexpr std::size_t text_vertices_per_quad = 6;

} // namespace cgpui
