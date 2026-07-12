#pragma once

#include "cgpui/ui/style_values.hpp"

namespace cgpui::detail {

[[nodiscard]] float tween_style_float(float from, float to, float progress);
[[nodiscard]] Size tween_style_size(Size from, Size to, float progress);
[[nodiscard]] Point tween_style_point(Point from, Point to, float progress);
[[nodiscard]] Rect tween_style_rect(Rect from, Rect to, float progress);
[[nodiscard]] EdgeSizes tween_style_edges(
    EdgeSizes from,
    EdgeSizes to,
    float progress);
[[nodiscard]] BorderRadii tween_style_radii(
    BorderRadii from,
    BorderRadii to,
    float progress);
[[nodiscard]] BoxShadow tween_style_shadow(
    BoxShadow from,
    BoxShadow to,
    float progress);
[[nodiscard]] PercentageSize tween_style_percentage_size(
    const PercentageSize& from,
    const PercentageSize& to,
    float progress);

} // namespace cgpui::detail
