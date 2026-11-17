#include "vulkan_backdrop_blur_resources_internal.hpp"

namespace cgpui {
namespace {

void destroy_pipeline(VkDevice device, VkPipeline& pipeline) {
  if (pipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, pipeline, nullptr);
  pipeline = VK_NULL_HANDLE;
}

void destroy_pass(VkDevice device, VkRenderPass& pass) {
  if (pass != VK_NULL_HANDLE) vkDestroyRenderPass(device, pass, nullptr);
  pass = VK_NULL_HANDLE;
}

} // namespace

Result<void> vulkan_prepare_backdrop_blur_resources(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkFormat format,
    VkExtent2D extent,
    VulkanBackdropBlurResources& resources) {
  if (resources.ready() && resources.format == format &&
      resources.extent.width == extent.width &&
      resources.extent.height == extent.height) {
    return {};
  }
  vulkan_destroy_backdrop_blur_resources(device, resources);
  Result<void> result = vulkan_create_backdrop_blur_targets(
      physical_device, device, format, extent, resources);
  if (result) result = vulkan_create_backdrop_blur_descriptors(device, resources);
  if (result) result = vulkan_create_backdrop_blur_pipelines(device, resources);
  if (!result) vulkan_destroy_backdrop_blur_resources(device, resources);
  return result;
}

void vulkan_destroy_backdrop_blur_resources(
    VkDevice device,
    VulkanBackdropBlurResources& resources) {
  if (device == VK_NULL_HANDLE) return;
  destroy_pipeline(device, resources.present_pipeline);
  destroy_pipeline(device, resources.composite_pipeline);
  destroy_pipeline(device, resources.blur_pipeline);
  destroy_pipeline(device, resources.downsample_pipeline);
  if (resources.pipeline_layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device, resources.pipeline_layout, nullptr);
  }
  if (resources.descriptor_pool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(device, resources.descriptor_pool, nullptr);
  }
  if (resources.set_layout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device, resources.set_layout, nullptr);
  }
  if (resources.sampler != VK_NULL_HANDLE) {
    vkDestroySampler(device, resources.sampler, nullptr);
  }
  for (VulkanBackdropBlurTarget& target : resources.targets) {
    if (target.framebuffer != VK_NULL_HANDLE) {
      vkDestroyFramebuffer(device, target.framebuffer, nullptr);
    }
    if (target.view != VK_NULL_HANDLE) {
      vkDestroyImageView(device, target.view, nullptr);
    }
    if (target.image != VK_NULL_HANDLE) {
      vkDestroyImage(device, target.image, nullptr);
    }
    if (target.memory != VK_NULL_HANDLE) {
      vkFreeMemory(device, target.memory, nullptr);
    }
  }
  destroy_pass(device, resources.blur_pass);
  destroy_pass(device, resources.scene_load_pass);
  destroy_pass(device, resources.scene_clear_pass);
  vulkan_destroy_frame_geometry_buffer(device, resources.vertices);
  resources = {};
}

} // namespace cgpui
