#include "vulkan_text_pipeline_internal.hpp"

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

int test_text_vertex_layout() {
  static_assert(std::is_standard_layout_v<cgpui::VulkanTextVertex>);
  static_assert(std::is_trivially_copyable_v<cgpui::VulkanTextVertex>);

  const VkVertexInputBindingDescription binding =
      cgpui::vulkan_text_vertex_binding_description();
  const auto attributes =
      cgpui::vulkan_text_vertex_attribute_descriptions();
  if (binding.binding != 0 ||
      binding.stride != sizeof(cgpui::VulkanTextVertex) ||
      binding.inputRate != VK_VERTEX_INPUT_RATE_VERTEX ||
      attributes.size() != 3) {
    return 10;
  }
  if (attributes[0].location != 0 ||
      attributes[0].format != VK_FORMAT_R32G32_SFLOAT ||
      attributes[0].offset != offsetof(cgpui::VulkanTextVertex, position) ||
      attributes[1].location != 1 ||
      attributes[1].format != VK_FORMAT_R32G32_SFLOAT ||
      attributes[1].offset != offsetof(cgpui::VulkanTextVertex, atlas_uv) ||
      attributes[2].location != 2 ||
      attributes[2].format != VK_FORMAT_R32G32B32A32_SFLOAT ||
      attributes[2].offset != offsetof(cgpui::VulkanTextVertex, color)) {
    return 11;
  }
  return 0;
}

int test_text_pipeline_fixed_state() {
  const VkPipelineInputAssemblyStateCreateInfo input_assembly =
      cgpui::vulkan_text_pipeline_input_assembly_state();
  const VkPipelineViewportStateCreateInfo viewport =
      cgpui::vulkan_text_pipeline_viewport_state();
  const VkPipelineRasterizationStateCreateInfo rasterization =
      cgpui::vulkan_text_pipeline_rasterization_state();
  const VkPipelineMultisampleStateCreateInfo multisample =
      cgpui::vulkan_text_pipeline_multisample_state();
  const VkPipelineDepthStencilStateCreateInfo depth_stencil =
      cgpui::vulkan_text_pipeline_depth_stencil_state();
  const VkPipelineColorBlendAttachmentState blend =
      cgpui::vulkan_text_pipeline_blend_attachment_state();
  const auto dynamic_states = cgpui::vulkan_text_pipeline_dynamic_states();
  constexpr VkColorComponentFlags color_write_mask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  if (input_assembly.sType !=
          VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO ||
      viewport.sType != VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO ||
      rasterization.sType !=
          VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO ||
      multisample.sType !=
          VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO ||
      depth_stencil.sType !=
          VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO ||
      input_assembly.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ||
      input_assembly.primitiveRestartEnable != VK_FALSE ||
      viewport.viewportCount != 1 || viewport.scissorCount != 1 ||
      rasterization.polygonMode != VK_POLYGON_MODE_FILL ||
      rasterization.cullMode != VK_CULL_MODE_NONE ||
      multisample.rasterizationSamples != VK_SAMPLE_COUNT_1_BIT ||
      depth_stencil.depthTestEnable != VK_FALSE ||
      depth_stencil.depthWriteEnable != VK_FALSE) {
    return 20;
  }
  if (blend.blendEnable != VK_TRUE ||
      blend.srcColorBlendFactor != VK_BLEND_FACTOR_SRC_ALPHA ||
      blend.dstColorBlendFactor != VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ||
      blend.colorBlendOp != VK_BLEND_OP_ADD ||
      blend.srcAlphaBlendFactor != VK_BLEND_FACTOR_ONE ||
      blend.dstAlphaBlendFactor != VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ||
      blend.alphaBlendOp != VK_BLEND_OP_ADD ||
      blend.colorWriteMask != color_write_mask ||
      dynamic_states[0] != VK_DYNAMIC_STATE_VIEWPORT ||
      dynamic_states[1] != VK_DYNAMIC_STATE_SCISSOR) {
    return 21;
  }
  return 0;
}

int test_text_pipeline_state_module_structure() {
  const std::string internal =
      read_source("src/renderer/vulkan/vulkan_text_pipeline_internal.hpp");
  const std::string state =
      read_source("src/renderer/vulkan/vulkan_text_pipeline_state.cpp");
  const std::string command_recording =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  if (internal.empty() || state.empty() || command_recording.empty()) {
    return 30;
  }
  if (!contains(internal, "struct VulkanTextVertex") ||
      !contains(state, "vulkan_text_vertex_binding_description(") ||
      !contains(state, "vulkan_text_pipeline_blend_attachment_state(") ||
      !contains(state, "VK_DYNAMIC_STATE_VIEWPORT") ||
      !contains(state, "VK_DYNAMIC_STATE_SCISSOR") ||
      contains(command_recording, "vulkan_text_pipeline_input_assembly_state(")) {
    return 31;
  }
  return 0;
}

int test_step_467_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 467",
      "VulkanTextVertex",
      "dynamic viewport/scissor",
      "straight alpha blending",
      "Step 468",
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
  if (const int result = test_text_vertex_layout(); result != 0) {
    return result;
  }
  if (const int result = test_text_pipeline_fixed_state(); result != 0) {
    return result;
  }
  if (const int result = test_text_pipeline_state_module_structure();
      result != 0) {
    return result;
  }
  return test_step_467_documentation();
}
