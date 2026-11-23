#include "vulkan_composition_transform_internal.hpp"
#include "cgpui/renderer/planning/composition_transform.hpp"

namespace cgpui {

AffineTransform vulkan_resolve_composed_transform(
    PaintMetadata metadata, const RendererCompositionStackRecord& stack) {
  return resolve_composed_transform(metadata, stack);
}
Point vulkan_transform_point(Point point, AffineTransform transform) {
  return transform_point(point, transform);
}
Point vulkan_apply_composed_transform(
    Point point, PaintMetadata metadata, const RendererCompositionStackRecord& stack) {
  return apply_composed_transform(point, metadata, stack);
}

} // namespace cgpui
