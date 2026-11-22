#include "vulkan_composition_presenter_internal.hpp"

#if defined(_WIN32)
#include "vulkan_device_internal.hpp"
#include "../win32_composition/win32_composition_presenter_internal.hpp"

namespace cgpui {

// The texture's memory, imported from its NT handle (the D3D11 side keeps
// the handle), bound to an image with the same format and extent.
Result<void> VulkanCompositionPresenter::import_texture() {
  const Win32CompositionSize size = composition_->size();
  extent_ = VkExtent2D{.width = size.width, .height = size.height};
  const VkExternalMemoryImageCreateInfo external{
      .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
      .handleTypes = vulkan_composition_handle_type};
  const VkImageCreateInfo image_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = &external,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = vulkan_composition_format,
      .extent = {.width = extent_.width, .height = extent_.height, .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = vulkan_composition_usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};
  if (auto result = require_vk_success(
          vkCreateImage(device_.device, &image_info, nullptr, &image_),
          "vkCreateImage for the composition texture failed");
      !result) {
    return result;
  }
  const HANDLE handle = composition_->shared_texture_handle();
  const auto handle_properties_of = reinterpret_cast<PFN_vkGetMemoryWin32HandlePropertiesKHR>(
      vkGetDeviceProcAddr(device_.device, "vkGetMemoryWin32HandlePropertiesKHR"));
  VkMemoryWin32HandlePropertiesKHR handle_properties{
      .sType = VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR};
  if (handle_properties_of == nullptr ||
      handle_properties_of(device_.device, vulkan_composition_handle_type, handle, &handle_properties) !=
          VK_SUCCESS) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "vkGetMemoryWin32HandlePropertiesKHR failed for the composition texture"));
  }
  VkMemoryRequirements requirements{};
  vkGetImageMemoryRequirements(device_.device, image_, &requirements);
  auto memory_type = vulkan_find_memory_type(
      device_.physical_device, requirements.memoryTypeBits & handle_properties.memoryTypeBits, 0);
  if (!memory_type) return std::unexpected(memory_type.error());
  const VkMemoryDedicatedAllocateInfo dedicated{
      .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO, .image = image_};
  const VkImportMemoryWin32HandleInfoKHR import{
      .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
      .pNext = &dedicated,
      .handleType = vulkan_composition_handle_type,
      .handle = handle};
  const VkMemoryAllocateInfo allocation{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = &import,
      .allocationSize = requirements.size,
      .memoryTypeIndex = *memory_type};
  if (auto result = require_vk_success(
          vkAllocateMemory(device_.device, &allocation, nullptr, &memory_),
          "vkAllocateMemory importing the composition texture failed");
      !result) {
    return result;
  }
  if (auto result = require_vk_success(
          vkBindImageMemory(device_.device, image_, memory_, 0),
          "vkBindImageMemory for the composition texture failed");
      !result) {
    return result;
  }
  return record_transfers();
}

void VulkanCompositionPresenter::release_texture() {
  if (transfers_[0] != VK_NULL_HANDLE) {
    vkFreeCommandBuffers(device_.device, device_.command_pool,
                         static_cast<std::uint32_t>(transfers_.size()), transfers_.data());
  }
  transfers_ = {};
  if (image_ != VK_NULL_HANDLE) vkDestroyImage(device_.device, image_, nullptr);
  if (memory_ != VK_NULL_HANDLE) vkFreeMemory(device_.device, memory_, nullptr);
  image_ = VK_NULL_HANDLE;
  memory_ = VK_NULL_HANDLE;
  extent_ = {};
}

} // namespace cgpui
#endif
