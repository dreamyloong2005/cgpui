#include "vulkan_rounded_rect_geometry_internal.hpp"
#include "cgpui/renderer/planning/rounded_rect_geometry.hpp"

namespace cgpui {

VulkanRoundedRectGeometry vulkan_build_rounded_rect_geometry(
    std::span<const RoundedRectDraw> rects, std::size_t segments,
    VulkanRoundedRectAntialiasingPolicy policy) {
  return build_rounded_rect_geometry(rects, segments, policy);
}

} // namespace cgpui
