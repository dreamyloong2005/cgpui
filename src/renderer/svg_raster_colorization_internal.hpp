#pragma once

#include "cgpui/renderer/svg_raster_colorization.hpp"

#include <string>

namespace cgpui::detail {

[[nodiscard]] std::string svg_current_color_css(
    const SvgRasterColorizationPlan& plan);

} // namespace cgpui::detail
