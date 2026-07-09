#include "vulkan_rounded_rect_pipeline_internal.hpp"

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

int test_rounded_rect_vertex_and_fixed_state() {
  static_assert(std::is_standard_layout_v<cgpui::VulkanRoundedRectVertex>);
  static_assert(std::is_trivially_copyable_v<cgpui::VulkanRoundedRectVertex>);
  const VkVertexInputBindingDescription binding =
      cgpui::vulkan_rounded_rect_vertex_binding_description();
  const auto attributes =
      cgpui::vulkan_rounded_rect_vertex_attribute_descriptions();
  if (binding.stride != sizeof(cgpui::VulkanRoundedRectVertex) ||
      binding.inputRate != VK_VERTEX_INPUT_RATE_VERTEX ||
      attributes.size() != 2 || attributes[0].location != 0 ||
      attributes[0].format != VK_FORMAT_R32G32_SFLOAT ||
      attributes[1].location != 1 ||
      attributes[1].format != VK_FORMAT_R32G32B32A32_SFLOAT) {
    return 10;
  }
  const VkPipelineInputAssemblyStateCreateInfo input =
      cgpui::vulkan_rounded_rect_input_assembly_state();
  const VkPipelineColorBlendAttachmentState blend =
      cgpui::vulkan_rounded_rect_blend_attachment_state();
  const auto dynamic_states = cgpui::vulkan_rounded_rect_dynamic_states();
  if (input.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ||
      blend.srcColorBlendFactor != VK_BLEND_FACTOR_SRC_ALPHA ||
      blend.dstColorBlendFactor != VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ||
      dynamic_states[0] != VK_DYNAMIC_STATE_VIEWPORT ||
      dynamic_states[1] != VK_DYNAMIC_STATE_SCISSOR) {
    return 11;
  }
  return 0;
}

int test_rounded_rect_shader_and_layout_contract() {
  static_assert(sizeof(cgpui::VulkanRoundedRectPushConstants) == 8);
  const std::span<const std::uint32_t> vertex =
      cgpui::vulkan_rounded_rect_vertex_shader_spirv();
  const std::span<const std::uint32_t> fragment =
      cgpui::vulkan_rounded_rect_fragment_shader_spirv();
  if (vertex.size() < 20 || fragment.size() < 20 ||
      vertex.front() != 0x07230203U || fragment.front() != 0x07230203U) {
    return 20;
  }
  const VkPushConstantRange range =
      cgpui::vulkan_rounded_rect_push_constant_range();
  const VkPipelineLayoutCreateInfo layout =
      cgpui::vulkan_rounded_rect_pipeline_layout_create_info(range);
  if (range.stageFlags != VK_SHADER_STAGE_VERTEX_BIT || range.offset != 0 ||
      range.size != sizeof(cgpui::VulkanRoundedRectPushConstants) ||
      layout.setLayoutCount != 0 || layout.pushConstantRangeCount != 1 ||
      layout.pPushConstantRanges != &range) {
    return 21;
  }
  cgpui::VulkanRoundedRectShaderModules modules{
      .vertex = fake_handle<VkShaderModule>(1),
      .fragment = fake_handle<VkShaderModule>(2),
  };
  const auto stages =
      cgpui::vulkan_rounded_rect_shader_stage_create_infos(modules);
  if (!modules.ready() || stages[0].module != modules.vertex ||
      stages[1].module != modules.fragment ||
      stages[0].stage != VK_SHADER_STAGE_VERTEX_BIT ||
      stages[1].stage != VK_SHADER_STAGE_FRAGMENT_BIT) {
    return 22;
  }
  return 0;
}

int test_rounded_rect_pipeline_resource_contract() {
  cgpui::VulkanRoundedRectPipelineResources resources;
  if (resources.ready()) {
    return 30;
  }
  resources.layout = fake_handle<VkPipelineLayout>(3);
  resources.pipeline = fake_handle<VkPipeline>(4);
  return resources.ready() ? 0 : 31;
}

int test_rounded_rect_pipeline_structure() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_pipeline_internal.hpp");
  const std::string state = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_pipeline_state.cpp");
  const std::string binaries = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_shader_binaries.cpp");
  const std::string modules = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_shader_modules.cpp");
  const std::string resources = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_pipeline_resources.cpp");
  const std::string vertex = read_source(
      "src/renderer/vulkan/shaders/rounded_rect.vert.glsl");
  const std::string fragment = read_source(
      "src/renderer/vulkan/shaders/rounded_rect.frag.glsl");
  const std::string swapchain =
      read_source("src/renderer/vulkan/vulkan_swapchain_internal.hpp");
  const std::string create =
      read_source("src/renderer/vulkan/vulkan_swapchain_create.cpp");
  const std::string lifecycle =
      read_source("src/renderer/vulkan/vulkan_swapchain_lifecycle.cpp");
  if (header.empty() || state.empty() || binaries.empty() || modules.empty() ||
      resources.empty() || vertex.empty() || fragment.empty() ||
      swapchain.empty() || create.empty() || lifecycle.empty()) {
    return 40;
  }
  if (!contains(header, "struct VulkanRoundedRectPipelineResources") ||
      !contains(state, "vulkan_rounded_rect_vertex_binding_description(") ||
      !contains(binaries, "vulkan_rounded_rect_vertex_shader_spirv(") ||
      !contains(modules, "vulkan_create_rounded_rect_shader_modules(") ||
      !contains(resources, "vkCreateGraphicsPipelines") ||
      !contains(vertex, "layout(push_constant)") ||
      !contains(fragment, "out_color = in_color") ||
      !contains(swapchain, "VulkanRoundedRectPipelineResources rounded_rect_pipeline") ||
      !contains(create, "vulkan_create_rounded_rect_pipeline_resources(") ||
      !contains(lifecycle, "vulkan_destroy_rounded_rect_pipeline_resources(")) {
    return 41;
  }
  return 0;
}

int test_step_477_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 477",
      "VulkanRoundedRectPipelineResources",
      "embedded rounded rectangle SPIR-V",
      "Step 478",
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
  if (const int result = test_rounded_rect_vertex_and_fixed_state();
      result != 0) {
    return result;
  }
  if (const int result = test_rounded_rect_shader_and_layout_contract();
      result != 0) {
    return result;
  }
  if (const int result = test_rounded_rect_pipeline_resource_contract();
      result != 0) {
    return result;
  }
  if (const int result = test_rounded_rect_pipeline_structure(); result != 0) {
    return result;
  }
  return test_step_477_documentation();
}
