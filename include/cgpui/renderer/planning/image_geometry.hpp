#pragma once

#include "cgpui/renderer/renderer_commands.hpp"
#include "cgpui/renderer/planning/image_vertex.hpp"

#include <span>
#include <vector>

namespace cgpui {

// Input bounds and composition translations are already in device pixels.
[[nodiscard]] std::vector<ImageVertex> build_image_vertices(
    std::span<const ImageDraw> image_draws);

} // namespace cgpui
