#include "vulkan_image_create_failure_test_support.hpp"

namespace cgpui_test {
namespace {
struct State {
  PFN_vkCreateImage create_image = nullptr;
  PFN_vkDestroyImage destroy_image = nullptr;
  PFN_vkDeviceWaitIdle wait_idle = nullptr;
  VulkanImageCreateFailureObservation observation;
  bool active = false;
} state;
VKAPI_ATTR VkResult VKAPI_CALL create_image(VkDevice device, const VkImageCreateInfo* info,
    const VkAllocationCallbacks* allocator, VkImage* image) {
  auto& o = state.observation;
  ++o.create_attempts;
  if (!o.injected) {
    o.injected = true;
    const auto idle = state.wait_idle(device);
    o.safely_quiesced = idle == VK_SUCCESS;
    *image = VK_NULL_HANDLE;
    return o.safely_quiesced ? VK_ERROR_OUT_OF_DEVICE_MEMORY : idle;
  }
  return state.create_image(device, info, allocator, image);
}
VKAPI_ATTR void VKAPI_CALL destroy_image(VkDevice device, VkImage image,
    const VkAllocationCallbacks* allocator) {
  if (image && !state.observation.injected) ++state.observation.images_destroyed_before_failure;
  state.destroy_image(device, image, allocator);
}
} // namespace
VulkanImageCreateFailureHook::VulkanImageCreateFailureHook() {
  if (state.active) return;
  state = {.create_image = cgpui::vkCreateImage, .destroy_image = cgpui::vkDestroyImage,
      .wait_idle = cgpui::vkDeviceWaitIdle, .active = true};
  if (!state.create_image || !state.destroy_image || !state.wait_idle) { state = {}; return; }
  cgpui::vkCreateImage = create_image;
  cgpui::vkDestroyImage = destroy_image;
  installed_ = true;
}
VulkanImageCreateFailureHook::~VulkanImageCreateFailureHook() {
  if (!installed_) return;
  cgpui::vkCreateImage = state.create_image;
  cgpui::vkDestroyImage = state.destroy_image;
  state.active = false;
}
bool VulkanImageCreateFailureHook::installed() const { return installed_; }
VulkanImageCreateFailureObservation VulkanImageCreateFailureHook::observation() const {
  return state.observation;
}
} // namespace cgpui_test
