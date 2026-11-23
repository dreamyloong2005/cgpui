#include "vulkan_rounded_rect_stroke_internal.hpp"
#include "../planning/rounded_rect_stroke_internal.hpp"

namespace cgpui {

VulkanRoundedRectStrokeResolution vulkan_resolve_rounded_rect_stroke(
    const RoundedRectDraw& draw, BorderRadii radii) {
  return resolve_rounded_rect_stroke(draw, radii);
}

} // namespace cgpui
