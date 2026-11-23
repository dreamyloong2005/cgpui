#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/renderer/planning/rect_vertex.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace cgpui {

struct RectDrawRange {
  std::size_t source_index = 0;
  std::size_t first_vertex = 0;
  std::size_t vertex_count = 0;
  std::size_t first_index = 0;
  std::size_t index_count = 0;
  std::optional<Rect> clip_rect;
};

struct RectGeometry {
  std::vector<RectVertex> vertices;
  std::vector<std::uint32_t> indices;
  std::vector<RectDrawRange> draws;
};

} // namespace cgpui
