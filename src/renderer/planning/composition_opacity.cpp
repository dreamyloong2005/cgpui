#include "cgpui/renderer/planning/composition_opacity.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {

float resolve_composed_opacity(
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack) {
  const float opacity = composition_stack.empty()
                            ? metadata.opacity
                            : composition_stack.current_metadata.opacity;
  return std::isfinite(opacity) ? std::clamp(opacity, 0.0F, 1.0F) : 1.0F;
}

Color apply_composed_opacity(
    Color color,
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack) {
  color.a *= resolve_composed_opacity(metadata, composition_stack);
  return color;
}

} // namespace cgpui
