#include "vulkan_glyph_atlas_resources_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {

void vulkan_destroy_glyph_atlas_page_resource(
    VkDevice device,
    VkDescriptorPool descriptor_pool,
    VulkanGlyphAtlasPageResource& resource) {
  if (resource.descriptor_set != VK_NULL_HANDLE &&
      descriptor_pool != VK_NULL_HANDLE) {
    (void)vkFreeDescriptorSets(
        device,
        descriptor_pool,
        1,
        &resource.descriptor_set);
    resource.descriptor_set = VK_NULL_HANDLE;
  }
  if (resource.image_view != VK_NULL_HANDLE) {
    vkDestroyImageView(device, resource.image_view, nullptr);
    resource.image_view = VK_NULL_HANDLE;
  }
  if (resource.image != VK_NULL_HANDLE) {
    vkDestroyImage(device, resource.image, nullptr);
    resource.image = VK_NULL_HANDLE;
  }
  if (resource.memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, resource.memory, nullptr);
    resource.memory = VK_NULL_HANDLE;
  }
}

void vulkan_destroy_glyph_atlas_resources(
    VkDevice device,
    VulkanGlyphAtlasResources& resources) {
  for (VulkanGlyphAtlasPageResource& page : resources.pages) {
    vulkan_destroy_glyph_atlas_page_resource(
        device,
        resources.descriptor_pool,
        page);
  }
  resources.pages.clear();
  if (resources.sampler != VK_NULL_HANDLE) {
    vkDestroySampler(device, resources.sampler, nullptr);
    resources.sampler = VK_NULL_HANDLE;
  }
  if (resources.descriptor_pool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(device, resources.descriptor_pool, nullptr);
    resources.descriptor_pool = VK_NULL_HANDLE;
  }
  if (resources.descriptor_set_layout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(
        device,
        resources.descriptor_set_layout,
        nullptr);
    resources.descriptor_set_layout = VK_NULL_HANDLE;
  }
}

bool vulkan_glyph_atlas_plan_fits_descriptor_capacity(
    const GlyphAtlasProductionPlan& plan) {
  return plan.live_resources.size() <=
         vulkan_glyph_atlas_descriptor_capacity;
}

Result<void> vulkan_update_glyph_atlas_resources(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const GlyphAtlasProductionPlan& plan,
    VulkanGlyphAtlasResources& resources) {
  if (!vulkan_glyph_atlas_plan_fits_descriptor_capacity(plan)) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "glyph atlas descriptor capacity exceeded"));
  }

  for (auto page = resources.pages.begin(); page != resources.pages.end();) {
    const bool live = std::ranges::any_of(
        plan.live_resources,
        [&](const GlyphAtlasProductionResourceRecord& record) {
          return record.page_index == page->page_index;
        });
    if (live) {
      ++page;
      continue;
    }
    vulkan_destroy_glyph_atlas_page_resource(
        device,
        resources.descriptor_pool,
        *page);
    page = resources.pages.erase(page);
  }

  for (const GlyphAtlasProductionResourceRecord& record : plan.live_resources) {
    auto page = std::ranges::find_if(
        resources.pages,
        [&](const VulkanGlyphAtlasPageResource& candidate) {
          return candidate.page_index == record.page_index;
        });
    const bool same_extent =
        page != resources.pages.end() &&
        std::isfinite(record.image.size.width) &&
        std::isfinite(record.image.size.height) &&
        std::ceil(record.image.size.width) ==
            static_cast<float>(page->extent.width) &&
        std::ceil(record.image.size.height) ==
            static_cast<float>(page->extent.height);
    if (same_extent) {
      page->generation = record.generation;
      continue;
    }
    if (page != resources.pages.end()) {
      vulkan_destroy_glyph_atlas_page_resource(
          device,
          resources.descriptor_pool,
          *page);
      resources.pages.erase(page);
    }

    auto created = vulkan_create_glyph_atlas_page_resource(
        physical_device,
        device,
        resources.descriptor_set_layout,
        resources.descriptor_pool,
        resources.sampler,
        record);
    if (!created) {
      return std::unexpected(created.error());
    }
    resources.pages.push_back(*created);
  }
  return {};
}

} // namespace cgpui
