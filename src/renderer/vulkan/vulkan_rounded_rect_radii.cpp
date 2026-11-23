#include "vulkan_rounded_rect_radii_internal.hpp"
#include "../planning/rounded_rect_radii_internal.hpp"

namespace cgpui {

VulkanRoundedRectRadiiResolution vulkan_resolve_rounded_rect_radii(Size size, BorderRadii radii) {
  return resolve_rounded_rect_radii(size, radii);
}

} // namespace cgpui
