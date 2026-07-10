#include "vulkan_image_texture_descriptors_internal.hpp"

#include "cgpui/renderer/image_sampling.hpp"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <type_traits>

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

template <typename Handle>
Handle fake_handle(std::uintptr_t value) {
  if constexpr (std::is_pointer_v<Handle>) {
    return reinterpret_cast<Handle>(value);
  } else {
    return static_cast<Handle>(value);
  }
}

int test_public_sampling_modes() {
  if (cgpui::image_sampling_mode_name(
          cgpui::ImageSamplingMode::nearest) != "nearest" ||
      cgpui::image_sampling_mode_name(
          cgpui::ImageSamplingMode::linear) != "linear") {
    return 10;
  }
  const cgpui::ImageDraw draw{};
  return draw.sampling == cgpui::ImageSamplingMode::linear ? 0 : 11;
}

int test_sampler_and_descriptor_create_info() {
  const VkSamplerCreateInfo nearest =
      cgpui::vulkan_image_texture_sampler_create_info(
          cgpui::ImageSamplingMode::nearest);
  const VkSamplerCreateInfo linear =
      cgpui::vulkan_image_texture_sampler_create_info(
          cgpui::ImageSamplingMode::linear);
  if (nearest.magFilter != VK_FILTER_NEAREST ||
      nearest.minFilter != VK_FILTER_NEAREST ||
      nearest.mipmapMode != VK_SAMPLER_MIPMAP_MODE_NEAREST ||
      linear.magFilter != VK_FILTER_LINEAR ||
      linear.minFilter != VK_FILTER_LINEAR ||
      linear.mipmapMode != VK_SAMPLER_MIPMAP_MODE_LINEAR ||
      nearest.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ||
      linear.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) {
    return 20;
  }

  const VkDescriptorSetLayoutBinding binding =
      cgpui::vulkan_image_texture_descriptor_layout_binding();
  const VkDescriptorPoolSize pool =
      cgpui::vulkan_image_texture_descriptor_pool_size();
  return binding.binding == 0 &&
                 binding.descriptorType ==
                     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER &&
                 binding.descriptorCount == 1 &&
                 binding.stageFlags == VK_SHADER_STAGE_FRAGMENT_BIT &&
                 pool.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER &&
                 pool.descriptorCount == 512
             ? 0
             : 21;
}

int test_descriptor_selection() {
  const VkDescriptorSet nearest = fake_handle<VkDescriptorSet>(41);
  const VkDescriptorSet linear = fake_handle<VkDescriptorSet>(42);
  cgpui::VulkanImageTextureResource resource{
      .nearest_descriptor_set = nearest,
      .linear_descriptor_set = linear,
  };
  return cgpui::vulkan_image_texture_descriptor_set(
             resource,
             cgpui::ImageSamplingMode::nearest) == nearest &&
                 cgpui::vulkan_image_texture_descriptor_set(
                     resource,
                     cgpui::ImageSamplingMode::linear) == linear
             ? 0
             : 30;
}

int test_structure_and_documentation() {
  const std::string public_header =
      read_source("include/cgpui/renderer/image_sampling.hpp");
  const std::string commands =
      read_source("include/cgpui/renderer/renderer_commands.hpp");
  const std::string private_header = read_source(
      "src/renderer/vulkan/vulkan_image_texture_descriptors_internal.hpp");
  const std::string samplers =
      read_source("src/renderer/vulkan/vulkan_image_texture_samplers.cpp");
  const std::string descriptors =
      read_source("src/renderer/vulkan/vulkan_image_texture_descriptors.cpp");
  const std::string resource_binding = read_source(
      "src/renderer/vulkan/vulkan_image_texture_resource_binding.cpp");
  const std::string images =
      read_source("src/renderer/vulkan/vulkan_image_texture_images.cpp");
  const std::string state_source =
      read_source("src/renderer/vulkan/vulkan_state.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (public_header.empty() || commands.empty() || private_header.empty() ||
      samplers.empty() || descriptors.empty() || resource_binding.empty() ||
      images.empty() || state_source.empty() || structure.empty()) {
    return 40;
  }
  if (!contains(public_header, "enum class ImageSamplingMode") ||
      !contains(commands, "ImageSamplingMode sampling") ||
      !contains(private_header,
                "vulkan_image_texture_descriptor_capacity = 256") ||
      !contains(samplers, "vkCreateSampler") ||
      !contains(descriptors, "vkCreateDescriptorSetLayout") ||
      !contains(descriptors, "vkCreateDescriptorPool") ||
      !contains(descriptors, "vkAllocateDescriptorSets") ||
      !contains(descriptors, "vkUpdateDescriptorSets") ||
      !contains(resource_binding,
                "vulkan_bind_image_texture_resource_descriptors(") ||
      !contains(images, "vkFreeDescriptorSets") ||
      !contains(state_source,
                "vulkan_create_image_texture_descriptor_resources(") ||
      !contains(structure, "vulkan_image_texture_descriptors.cpp")) {
    return 41;
  }

  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 493",
      "ImageSamplingMode",
      "nearest/linear samplers",
      "descriptor set binding",
      "Step 494",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 50;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_public_sampling_modes(); result != 0) {
    return result;
  }
  if (const int result = test_sampler_and_descriptor_create_info();
      result != 0) {
    return result;
  }
  if (const int result = test_descriptor_selection(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
