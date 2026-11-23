#include "vulkan_rounded_rect_antialiasing_internal.hpp"
#include "cgpui/renderer/planning/rect_antialiasing.hpp"

namespace cgpui {

VulkanRoundedRectAntialiasingPolicy vulkan_default_rounded_rect_antialiasing_policy() {
  return default_rounded_rect_antialiasing_policy();
}
float vulkan_rounded_rect_coverage(float coverage) {
  return rounded_rect_coverage(coverage);
}

} // namespace cgpui
