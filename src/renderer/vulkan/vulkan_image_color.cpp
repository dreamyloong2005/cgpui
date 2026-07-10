#include "vulkan_image_color_internal.hpp"

#include "vulkan_composition_opacity_internal.hpp"

namespace cgpui {

Color vulkan_resolve_image_tint(const ImageDraw& draw) {
  const Color tint = draw.tint.value_or(
      Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F});
  return vulkan_apply_composed_opacity(
      tint, draw.metadata, draw.composition_stack);
}

} // namespace cgpui
