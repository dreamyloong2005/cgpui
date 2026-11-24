#include "vulkan_state_internal.hpp"

namespace cgpui {

std::vector<VulkanFrameCommandReuseState> VulkanRendererState::prepare_swapchain_install(
    VulkanSwapchainResources& resources) {
  // Complete the only allocating install step before transferring any native
  // handle/vector into state. Failed candidates still belong to this device.
  try {
    return std::vector<VulkanFrameCommandReuseState>(resources.command_buffers.size());
  } catch (...) {
    destroy_swapchain_resources(resources);
    throw;
  }
}

} // namespace cgpui
