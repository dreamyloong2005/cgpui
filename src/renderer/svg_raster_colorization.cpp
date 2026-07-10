#include "cgpui/renderer/svg_raster_colorization.hpp"

#include "svg_raster_colorization_internal.hpp"

#include <cmath>
#include <cstdio>

namespace cgpui {
namespace {

bool valid_color_component(float component) {
  return std::isfinite(component) && component >= 0.0F && component <= 1.0F;
}

std::uint8_t color_byte(float component) {
  return static_cast<std::uint8_t>(std::lround(component * 255.0F));
}

} // namespace

bool SvgRasterColorizationPlan::ready() const {
  return status == SvgRasterColorizationStatus::ready;
}

SvgRasterColorizationPlan plan_svg_raster_colorization(
    const SvgRasterColorizationRequest& request) {
  SvgRasterColorizationPlan plan;
  if (!request.current_color.has_value()) {
    return plan;
  }

  const Color color = *request.current_color;
  if (!valid_color_component(color.r) || !valid_color_component(color.g) ||
      !valid_color_component(color.b) || !valid_color_component(color.a)) {
    plan.status = SvgRasterColorizationStatus::invalid_color;
    return plan;
  }

  plan.recolors_current_color = true;
  plan.rgba8 = {
      color_byte(color.r),
      color_byte(color.g),
      color_byte(color.b),
      color_byte(color.a),
  };
  return plan;
}

} // namespace cgpui

namespace cgpui::detail {

std::string svg_current_color_css(const SvgRasterColorizationPlan& plan) {
  char css[48]{};
  const double alpha = static_cast<double>(plan.rgba8[3]) / 255.0;
  std::snprintf(
      css,
      sizeof(css),
      "rgba(%u,%u,%u,%.9g)",
      static_cast<unsigned>(plan.rgba8[0]),
      static_cast<unsigned>(plan.rgba8[1]),
      static_cast<unsigned>(plan.rgba8[2]),
      alpha);
  return css;
}

} // namespace cgpui::detail
