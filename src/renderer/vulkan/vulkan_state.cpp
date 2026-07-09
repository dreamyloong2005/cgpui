#include "vulkan_internal.hpp"

namespace cgpui {

VulkanRendererState::VulkanRendererState(RenderSurfaceDescriptor descriptor)
    : descriptor_(std::move(descriptor)) {}

VulkanRendererState::~VulkanRendererState() {
  if (device_ != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(device_);
    destroy_swapchain();
    destroy_sync_objects();
    vulkan_destroy_glyph_atlas_resources(device_, glyph_atlas_resources_);
    if (command_pool_ != VK_NULL_HANDLE) {
      vkDestroyCommandPool(device_, command_pool_, nullptr);
    }
    vkDestroyDevice(device_, nullptr);
  }
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
  if (auto result = renderer->create_instance(); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = renderer->create_surface(); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = renderer->select_physical_device(); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = renderer->create_device(); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = renderer->create_command_pool(); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = vulkan_create_glyph_atlas_descriptor_resources(
          renderer->device_,
          renderer->glyph_atlas_resources_);
      !result) {
    return std::unexpected(result.error());
  }
  if (auto result = renderer->create_swapchain(); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = renderer->create_sync_objects(); !result) {
    return std::unexpected(result.error());
  }

  return renderer;
}

} // namespace cgpui
