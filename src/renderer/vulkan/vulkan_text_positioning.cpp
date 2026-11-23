#include "vulkan_text_positioning_internal.hpp"

namespace cgpui {

VulkanTextPositioningPolicy vulkan_default_text_positioning_policy() {
  return default_text_positioning_policy();
}

Rect vulkan_position_text_bounds(Rect bounds, VulkanTextPositioningPolicy policy) {
  return position_text_bounds(bounds, policy);
}

} // namespace cgpui
