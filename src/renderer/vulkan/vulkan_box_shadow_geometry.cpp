#include "vulkan_box_shadow_geometry_internal.hpp"

namespace cgpui {

float vulkan_box_shadow_sigma(float blur_radius) {
  return box_shadow_sigma(blur_radius);
}

float vulkan_box_shadow_spread_radius(float radius, float spread) {
  return box_shadow_spread_radius(radius, spread);
}

VulkanBoxShadowGeometry vulkan_build_box_shadow_geometry(
    std::span<const BoxShadowDraw> shadows) {
  return build_box_shadow_geometry(shadows);
}

} // namespace cgpui
