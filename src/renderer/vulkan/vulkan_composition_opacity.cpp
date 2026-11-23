#include "vulkan_composition_opacity_internal.hpp"
#include "cgpui/renderer/planning/composition_opacity.hpp"

namespace cgpui {

float vulkan_resolve_composed_opacity(
    PaintMetadata metadata, const RendererCompositionStackRecord& stack) {
  return resolve_composed_opacity(metadata, stack);
}
Color vulkan_apply_composed_opacity(
    Color color, PaintMetadata metadata, const RendererCompositionStackRecord& stack) {
  return apply_composed_opacity(color, metadata, stack);
}

} // namespace cgpui
