#pragma once

#include "cgpui/core/geometry.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace cgpui {

enum class SvgRasterColorizationStatus {
  ready,
  invalid_color,
};

struct SvgRasterColorizationRequest {
  std::optional<Color> current_color;
};

struct SvgRasterColorizationPlan {
  SvgRasterColorizationStatus status = SvgRasterColorizationStatus::ready;
  bool recolors_current_color = false;
  std::array<std::uint8_t, 4> rgba8{0, 0, 0, 255};

  [[nodiscard]] bool ready() const;

  friend bool operator==(
      const SvgRasterColorizationPlan&,
      const SvgRasterColorizationPlan&) = default;
};

[[nodiscard]] SvgRasterColorizationPlan plan_svg_raster_colorization(
    const SvgRasterColorizationRequest& request);

} // namespace cgpui
