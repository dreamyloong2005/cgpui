#include "vulkan_composition_presenter_internal.hpp"

// DirectComposition presentation exists on Windows only; elsewhere no
// device is suitable and the presenter reports that.
#if !defined(_WIN32)
namespace cgpui {
namespace {

Error unsupported() {
  return vulkan_error(
      ErrorCode::unsupported_platform, "DirectComposition presentation requires Windows");
}

} // namespace

std::span<const char* const> vulkan_composition_device_extensions() { return {}; }

bool vulkan_composition_device_suitable(VkPhysicalDevice) { return false; }

Result<std::unique_ptr<VulkanCompositionPresenter>> VulkanCompositionPresenter::create(
    Device, const Win32SurfaceHandle&, VkExtent2D) {
  return std::unexpected(unsupported());
}

VulkanCompositionPresenter::~VulkanCompositionPresenter() = default;

Result<void> VulkanCompositionPresenter::resize(VkExtent2D) {
  return std::unexpected(unsupported());
}

VkResult VulkanCompositionPresenter::submit(VkQueue, VkCommandBuffer, VkFence) const {
  return VK_ERROR_FEATURE_NOT_PRESENT;
}

Result<void> VulkanCompositionPresenter::present() { return std::unexpected(unsupported()); }

} // namespace cgpui
#endif
