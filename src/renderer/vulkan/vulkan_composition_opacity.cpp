#include "vulkan_composition_opacity_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {

float vulkan_resolve_composed_opacity(
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack) {
  const float opacity = composition_stack.empty()
                            ? metadata.opacity
                            : composition_stack.current_metadata.opacity;
  return std::isfinite(opacity) ? std::clamp(opacity, 0.0F, 1.0F) : 1.0F;
}

Color vulkan_apply_composed_opacity(
    Color color,
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack) {
  color.a *= vulkan_resolve_composed_opacity(metadata, composition_stack);
  return color;
}

} // namespace cgpui
