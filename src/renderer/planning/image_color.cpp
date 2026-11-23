#include "cgpui/renderer/planning/image_color.hpp"

#include "cgpui/renderer/planning/composition_opacity.hpp"

namespace cgpui {

Color resolve_image_tint(const ImageDraw& draw) {
  const Color tint = draw.tint.value_or(
      Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F});
  return apply_composed_opacity(
      tint, draw.metadata, draw.composition_stack);
}

} // namespace cgpui
