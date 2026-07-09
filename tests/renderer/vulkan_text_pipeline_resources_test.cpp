#include "vulkan_text_pipeline_resources_internal.hpp"

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

int test_pipeline_resource_contract() {
  static_assert(sizeof(cgpui::VulkanTextPushConstants) == 8);
  static_assert(std::is_standard_layout_v<cgpui::VulkanTextPushConstants>);

  cgpui::VulkanTextPipelineResources resources;
  if (resources.ready()) {
    return 10;
  }
  resources.layout = fake_handle<VkPipelineLayout>(1);
  resources.pipeline = fake_handle<VkPipeline>(2);
  if (!resources.ready()) {
    return 11;
  }
  return 0;
}

int test_pipeline_layout_contract() {
  const VkPushConstantRange push_constants =
      cgpui::vulkan_text_push_constant_range();
  if (push_constants.stageFlags != VK_SHADER_STAGE_VERTEX_BIT ||
      push_constants.offset != 0 ||
      push_constants.size != sizeof(cgpui::VulkanTextPushConstants)) {
    return 20;
  }

  const VkDescriptorSetLayout descriptor_set_layout =
      fake_handle<VkDescriptorSetLayout>(3);
  const VkPipelineLayoutCreateInfo create_info =
      cgpui::vulkan_text_pipeline_layout_create_info(
          descriptor_set_layout,
          push_constants);
  if (create_info.sType != VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO ||
      create_info.setLayoutCount != 1 ||
      create_info.pSetLayouts != &descriptor_set_layout ||
      create_info.pushConstantRangeCount != 1 ||
      create_info.pPushConstantRanges != &push_constants) {
    return 21;
  }
  return 0;
}

int test_pipeline_resource_structure() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_text_pipeline_resources_internal.hpp");
  const std::string source = read_source(
      "src/renderer/vulkan/vulkan_text_pipeline_resources.cpp");
  const std::string swapchain =
      read_source("src/renderer/vulkan/vulkan_swapchain_internal.hpp");
  const std::string create =
      read_source("src/renderer/vulkan/vulkan_swapchain_create.cpp");
  const std::string lifecycle =
      read_source("src/renderer/vulkan/vulkan_swapchain_lifecycle.cpp");
  if (header.empty() || source.empty() || swapchain.empty() ||
      create.empty() || lifecycle.empty()) {
    return 30;
  }
  if (!contains(header, "struct VulkanTextPipelineResources") ||
      !contains(source, "vkCreatePipelineLayout") ||
      !contains(source, "vkCreateGraphicsPipelines") ||
      !contains(source, "vulkan_destroy_text_shader_modules") ||
      !contains(swapchain, "VulkanTextPipelineResources text_pipeline") ||
      !contains(create, "vulkan_create_text_pipeline_resources(") ||
      !contains(lifecycle, "vulkan_destroy_text_pipeline_resources(")) {
    return 31;
  }
  return 0;
}

int test_step_469_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 469",
      "VulkanTextPipelineResources",
      "vulkan_create_text_pipeline_resources",
      "Step 470",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 40;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_pipeline_resource_contract(); result != 0) {
    return result;
  }
  if (const int result = test_pipeline_layout_contract(); result != 0) {
    return result;
  }
  if (const int result = test_pipeline_resource_structure(); result != 0) {
    return result;
  }
  return test_step_469_documentation();
}
