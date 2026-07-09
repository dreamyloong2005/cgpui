#include "vulkan_glyph_atlas_resources_internal.hpp"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }
  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

int test_descriptor_layout_and_write_shapes() {
  const VkDescriptorSetLayoutBinding binding =
      cgpui::vulkan_glyph_atlas_descriptor_layout_binding();
  if (binding.binding != 0 || binding.descriptorCount != 1 ||
      binding.descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
      binding.stageFlags != VK_SHADER_STAGE_FRAGMENT_BIT) {
    return 10;
  }

  const VkDescriptorPoolSize pool_size =
      cgpui::vulkan_glyph_atlas_descriptor_pool_size(8);
  if (pool_size.type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
      pool_size.descriptorCount != 8) {
    return 11;
  }

  const VkDescriptorImageInfo image_info =
      cgpui::vulkan_glyph_atlas_descriptor_image_info(
          VK_NULL_HANDLE,
          VK_NULL_HANDLE);
  if (image_info.imageLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    return 12;
  }

  const VkWriteDescriptorSet write =
      cgpui::vulkan_glyph_atlas_descriptor_write(
          VK_NULL_HANDLE,
          image_info);
  return write.sType == VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET &&
                 write.dstBinding == 0 && write.descriptorCount == 1 &&
                 write.descriptorType ==
                     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER &&
                 write.pImageInfo == &image_info
             ? 0
             : 13;
}

int test_private_resource_ownership_structure() {
  const std::string internal = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_resources_internal.hpp");
  const std::string descriptors = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_descriptors.cpp");
  const std::string images = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_images.cpp");
  const std::string resources = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_resources.cpp");
  const std::string state =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_frame.cpp");
  const std::string public_production =
      read_source("include/cgpui/renderer/glyph_atlas_production.hpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");

  if (internal.empty() || descriptors.empty() || images.empty() ||
      resources.empty() || state.empty() || frame.empty() ||
      public_production.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty()) {
    return 20;
  }
  if (!contains(internal, "struct VulkanGlyphAtlasPageResource") ||
      !contains(internal, "struct VulkanGlyphAtlasResources") ||
      !contains(internal, "VkDescriptorSetLayout descriptor_set_layout") ||
      !contains(internal, "VkDescriptorPool descriptor_pool") ||
      !contains(internal, "VkDescriptorSet descriptor_set") ||
      !contains(internal, "VkImage image") ||
      !contains(internal, "VkDeviceMemory memory") ||
      !contains(internal, "VkImageView image_view") ||
      !contains(internal, "VkSampler sampler")) {
    return 21;
  }
  if (!contains(descriptors, "vkCreateDescriptorSetLayout") ||
      !contains(descriptors, "vkCreateDescriptorPool") ||
      !contains(descriptors, "vkAllocateDescriptorSets") ||
      !contains(descriptors, "vkUpdateDescriptorSets") ||
      !contains(images, "vkCreateImage") ||
      !contains(images, "vkAllocateMemory") ||
      !contains(images, "vkBindImageMemory") ||
      !contains(images, "vkCreateImageView") ||
      !contains(images, "vkCreateSampler")) {
    return 22;
  }
  if (!contains(resources, "vulkan_update_glyph_atlas_resources(") ||
      !contains(state, "VulkanGlyphAtlasResources glyph_atlas_resources_") ||
      !contains(state,
                "GlyphAtlasProductionResourceState glyph_atlas_plan_state_") ||
      !contains(frame, "vulkan_plan_glyph_atlas_production_resources(") ||
      !contains(frame, "vulkan_update_glyph_atlas_resources(")) {
    return 23;
  }
  return contains(public_production, "VkImage") ||
                 contains(public_production, "VkDescriptorSet") ||
                 contains(public_production, "vulkan.h")
             ? 24
             : 0;
}

int test_step_460_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 460",
      "VulkanGlyphAtlasResources",
      "vulkan_glyph_atlas_descriptors.cpp",
      "R8_UNORM",
      "vkUpdateDescriptorSets",
      "Step 461",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 30;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_descriptor_layout_and_write_shapes(); result != 0) {
    return result;
  }
  if (const int result = test_private_resource_ownership_structure(); result != 0) {
    return result;
  }
  return test_step_460_documentation();
}
