#include "vulkan_internal.hpp"

namespace cgpui {

VulkanRendererState::VulkanRendererState(RenderSurfaceDescriptor descriptor)
    : descriptor_(std::move(descriptor)) {}

VulkanRendererState::~VulkanRendererState() {
  destroy_device_resources(true);
  if (surface_ != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
  }
  if (instance_ != VK_NULL_HANDLE) {
    vkDestroyInstance(instance_, nullptr);
  }
}

Result<std::shared_ptr<VulkanRendererState>> VulkanRendererState::create(
    RenderSurfaceDescriptor descriptor) {
  auto renderer = std::shared_ptr<VulkanRendererState>(
      new VulkanRendererState(std::move(descriptor)));
  if (auto result = renderer->initialize_device_selection(); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = renderer->create_instance(); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = renderer->create_surface(); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = renderer->select_physical_device(); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = renderer->create_device_resources(); !result) {
    return std::unexpected(result.error());
  }
  renderer->device_generation_ = 1;
  return renderer;
}

} // namespace cgpui
