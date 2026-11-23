#include "vulkan_solid_rect_geometry_internal.hpp"
#include "cgpui/renderer/planning/solid_rect_geometry.hpp"

namespace cgpui {

VulkanRoundedRectGeometry vulkan_build_solid_rect_geometry(std::span<const SolidRect> rects) {
  return build_solid_rect_geometry(rects);
}

} // namespace cgpui
