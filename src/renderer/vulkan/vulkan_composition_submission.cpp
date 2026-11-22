#include "vulkan_composition_presenter_internal.hpp"

#if defined(_WIN32)
#include "../win32_composition/win32_composition_presenter_internal.hpp"

namespace cgpui {
namespace {

// How long the queue waits for D3D11 to give the render key back; it does
// right after its copy in present().
constexpr std::uint32_t render_key_timeout_ms = 1000;

Result<void> record_transfer(VkCommandBuffer command_buffer, const VkImageMemoryBarrier& barrier,
                             VkPipelineStageFlags source, VkPipelineStageFlags destination) {
  const VkCommandBufferBeginInfo begin{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  if (auto result = require_vk_success(
          vkBeginCommandBuffer(command_buffer, &begin),
          "vkBeginCommandBuffer for a composition transfer failed");
      !result) {
    return result;
  }
  vkCmdPipelineBarrier(command_buffer, source, destination, 0, 0, nullptr, 0, nullptr, 1,
                       &barrier);
  return require_vk_success(
      vkEndCommandBuffer(command_buffer), "vkEndCommandBuffer for a composition transfer failed");
}

} // namespace

// Around every frame: the image comes from D3D11 (contents discarded; the
// frame clears it) and goes back to it after the frame's render pass and
// any pixel capture, which leave it in vulkan_composition_frame_layout.
Result<void> VulkanCompositionPresenter::record_transfers() {
  const VkCommandBufferAllocateInfo allocation{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = device_.command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<std::uint32_t>(transfers_.size())};
  if (auto result = require_vk_success(
          vkAllocateCommandBuffers(device_.device, &allocation, transfers_.data()),
          "vkAllocateCommandBuffers for composition transfers failed");
      !result) {
    transfers_ = {};
    return result;
  }
  const VkImageSubresourceRange color{
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1};
  const VkImageMemoryBarrier acquire{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = vulkan_composition_frame_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL,
      .dstQueueFamilyIndex = device_.queue_family,
      .image = image_,
      .subresourceRange = color};
  const VkImageMemoryBarrier release{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = vulkan_composition_frame_layout,
      .newLayout = VK_IMAGE_LAYOUT_GENERAL,
      .srcQueueFamilyIndex = device_.queue_family,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL,
      .image = image_,
      .subresourceRange = color};
  if (auto result = record_transfer(transfers_[0], acquire, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
      !result) {
    return result;
  }
  return record_transfer(transfers_[1], release, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
}

Result<void> VulkanCompositionPresenter::submit(
    VkQueue queue, VkCommandBuffer frame, VkFence fence) const {
  const std::array<VkCommandBuffer, 3> command_buffers{transfers_[0], frame, transfers_[1]};
  const std::uint64_t render_key = win32_composition_render_key;
  const std::uint64_t present_key = win32_composition_present_key;
  const std::uint32_t timeout = render_key_timeout_ms;
  const VkWin32KeyedMutexAcquireReleaseInfoKHR keyed_mutex{
      .sType = VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR,
      .acquireCount = 1,
      .pAcquireSyncs = &memory_,
      .pAcquireKeys = &render_key,
      .pAcquireTimeouts = &timeout,
      .releaseCount = 1,
      .pReleaseSyncs = &memory_,
      .pReleaseKeys = &present_key};
  const VkSubmitInfo submit_info{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = &keyed_mutex,
      .commandBufferCount = static_cast<std::uint32_t>(command_buffers.size()),
      .pCommandBuffers = command_buffers.data()};
  return require_vk_success(
      vkQueueSubmit(queue, 1, &submit_info, fence),
      "vkQueueSubmit under the composition keyed mutex failed");
}

Result<void> VulkanCompositionPresenter::present() { return composition_->present(); }

} // namespace cgpui
#endif
