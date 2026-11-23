#include "cgpui/renderer/planning/composition_transform.hpp"

#include <cmath>

namespace cgpui {
namespace {

[[nodiscard]] bool finite_transform(AffineTransform transform) {
  return std::isfinite(transform.scale_x) &&
         std::isfinite(transform.skew_y) &&
         std::isfinite(transform.skew_x) &&
         std::isfinite(transform.scale_y) &&
         std::isfinite(transform.translate_x) &&
         std::isfinite(transform.translate_y);
}

} // namespace

AffineTransform resolve_composed_transform(
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack) {
  const AffineTransform transform = composition_stack.empty()
                                        ? metadata.transform
                                        : composition_stack.current_metadata.transform;
  return finite_transform(transform) ? transform : AffineTransform::identity();
}

Point transform_point(Point point, AffineTransform transform) {
  return Point{
      .x = transform.scale_x * point.x + transform.skew_x * point.y +
           transform.translate_x,
      .y = transform.skew_y * point.x + transform.scale_y * point.y +
           transform.translate_y,
  };
}

Point apply_composed_transform(
    Point point,
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack) {
  return transform_point(
      point, resolve_composed_transform(metadata, composition_stack));
}

} // namespace cgpui
