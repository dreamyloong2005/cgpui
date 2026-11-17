#pragma once

#include "vulkan_backdrop_blur_plan_internal.hpp"
#include "vulkan_frame_geometry_buffer_internal.hpp"

#include <array>

namespace cgpui {

// Offscreen target in the swapchain format, sampled by the effect shaders.
struct VulkanBackdropBlurTarget {
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VkFramebuffer framebuffer = VK_NULL_HANDLE;
  VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
};

inline constexpr std::size_t kVulkanBackdropSceneTarget = 0;
inline constexpr std::size_t kVulkanBackdropPingTarget = 1;
inline constexpr std::size_t kVulkanBackdropPongTarget = 2;

// Created on the first frame with a backdrop blur and kept until the
// swapchain format or extent changes; frames without one never touch it.
struct VulkanBackdropBlurResources {
  VkFormat format = VK_FORMAT_UNDEFINED;
  VkExtent2D extent{};
  VkRenderPass scene_clear_pass = VK_NULL_HANDLE;
  VkRenderPass scene_load_pass = VK_NULL_HANDLE;
  VkRenderPass blur_pass = VK_NULL_HANDLE;
  std::array<VulkanBackdropBlurTarget, 3> targets{};
  VkSampler sampler = VK_NULL_HANDLE;
  VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;
  VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
  VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
  VkPipeline downsample_pipeline = VK_NULL_HANDLE;
  VkPipeline blur_pipeline = VK_NULL_HANDLE;
  VkPipeline composite_pipeline = VK_NULL_HANDLE;
  VkPipeline present_pipeline = VK_NULL_HANDLE;
  VulkanFrameGeometryBufferResources vertices;

  [[nodiscard]] bool ready() const {
    return present_pipeline != VK_NULL_HANDLE &&
           targets[kVulkanBackdropPongTarget].descriptor_set != VK_NULL_HANDLE;
  }
};

// Shared by every effect shader (each reads the fields it needs).
struct VulkanBackdropBlurPushConstants {
  std::array<float, 2> framebuffer_size{};
  std::array<std::int32_t, 2> source_origin{};
  std::array<std::int32_t, 2> source_max{};
  std::array<std::int32_t, 2> direction{};
  std::int32_t factor = 1;
  std::int32_t radius = 0;
  float sigma = 0.0F;
  float padding = 0.0F;
};

Result<void> vulkan_create_backdrop_blur_targets(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkFormat format,
    VkExtent2D extent,
    VulkanBackdropBlurResources& resources);
Result<void> vulkan_create_backdrop_blur_descriptors(
    VkDevice device,
    VulkanBackdropBlurResources& resources);
Result<void> vulkan_create_backdrop_blur_pipelines(
    VkDevice device,
    VulkanBackdropBlurResources& resources);
// Recreates everything when the format or extent differs.
Result<void> vulkan_prepare_backdrop_blur_resources(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkFormat format,
    VkExtent2D extent,
    VulkanBackdropBlurResources& resources);
void vulkan_destroy_backdrop_blur_resources(
    VkDevice device,
    VulkanBackdropBlurResources& resources);

[[nodiscard]] std::span<const std::uint32_t>
vulkan_backdrop_blur_shader_spirv(std::size_t index);

} // namespace cgpui
