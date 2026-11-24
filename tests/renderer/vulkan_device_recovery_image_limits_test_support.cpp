#include "vulkan_device_recovery_image_limits_test_support.hpp"

namespace cgpui_test {
namespace {
struct Limits {
  PFN_vkGetPhysicalDeviceProperties properties = nullptr;
  PFN_vkCreateImage create_image = nullptr;
  VkPhysicalDevice physical = VK_NULL_HANDLE;
  std::uint32_t maximum = 0;
  unsigned overrides = 0, attempts = 0, invalid_attempts = 0;
  bool active = false;
} state;
VKAPI_ATTR void VKAPI_CALL properties(VkPhysicalDevice physical, VkPhysicalDeviceProperties* output) {
  state.properties(physical, output);
  if (state.maximum && physical == state.physical) {
    ++state.overrides;
    output->limits.maxImageDimension2D = state.maximum;
  }
}
VKAPI_ATTR VkResult VKAPI_CALL create_image(VkDevice device, const VkImageCreateInfo* info,
    const VkAllocationCallbacks* allocator, VkImage* image) {
  ++state.attempts;
  if (state.maximum && (info->extent.width > state.maximum || info->extent.height > state.maximum)) {
    ++state.invalid_attempts;
    *image = VK_NULL_HANDLE;
    // A broken preflight must fail the assertion, not issue an invalid native
    // create against the healthy real driver used by this simulated-limit test.
    return VK_ERROR_FORMAT_NOT_SUPPORTED;
  }
  return state.create_image(device, info, allocator, image);
}
} // namespace
VulkanImageLimitHook::VulkanImageLimitHook(VkPhysicalDevice physical, std::uint32_t maximum) {
  if (state.active || !physical || !maximum) return;
  state = {.properties = cgpui::vkGetPhysicalDeviceProperties, .create_image = cgpui::vkCreateImage,
      .physical = physical, .maximum = maximum, .active = true};
  if (!state.properties || !state.create_image) { state = {}; return; }
  cgpui::vkGetPhysicalDeviceProperties = properties;
  cgpui::vkCreateImage = create_image;
  installed_ = true;
}
VulkanImageLimitHook::~VulkanImageLimitHook() {
  if (!installed_) return;
  cgpui::vkGetPhysicalDeviceProperties = state.properties;
  cgpui::vkCreateImage = state.create_image;
  state.active = false;
}
void VulkanImageLimitHook::restore_properties() { if (installed_) state.maximum = 0; }
bool VulkanImageLimitHook::installed() const { return installed_; }
unsigned VulkanImageLimitHook::property_overrides() const { return state.overrides; }
unsigned VulkanImageLimitHook::image_attempts() const { return state.attempts; }
unsigned VulkanImageLimitHook::invalid_image_attempts() const { return state.invalid_attempts; }
} // namespace cgpui_test
