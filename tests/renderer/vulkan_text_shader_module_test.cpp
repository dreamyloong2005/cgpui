#include "vulkan_text_pipeline_internal.hpp"

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

int test_embedded_spirv_binaries() {
  constexpr std::uint32_t spirv_magic = 0x07230203U;
  const std::span<const std::uint32_t> vertex =
      cgpui::vulkan_text_vertex_shader_spirv();
  const std::span<const std::uint32_t> fragment =
      cgpui::vulkan_text_fragment_shader_spirv();
  if (vertex.size() < 20 || fragment.size() < 20 ||
      vertex.front() != spirv_magic || fragment.front() != spirv_magic) {
    return 10;
  }

  const VkShaderModuleCreateInfo vertex_info =
      cgpui::vulkan_text_shader_module_create_info(vertex);
  const VkShaderModuleCreateInfo fragment_info =
      cgpui::vulkan_text_shader_module_create_info(fragment);
  if (vertex_info.sType != VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO ||
      vertex_info.codeSize != vertex.size_bytes() ||
      vertex_info.pCode != vertex.data() ||
      fragment_info.codeSize != fragment.size_bytes() ||
      fragment_info.pCode != fragment.data()) {
    return 11;
  }
  return 0;
}

int test_shader_stage_contract() {
  cgpui::VulkanTextShaderModules modules{
      .vertex = fake_handle<VkShaderModule>(1),
      .fragment = fake_handle<VkShaderModule>(2),
  };
  if (!modules.ready()) {
    return 20;
  }
  const auto stages = cgpui::vulkan_text_shader_stage_create_infos(modules);
  if (stages[0].sType !=
          VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO ||
      stages[0].stage != VK_SHADER_STAGE_VERTEX_BIT ||
      stages[0].module != modules.vertex ||
      std::string{stages[0].pName} != "main" ||
      stages[1].sType !=
          VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO ||
      stages[1].stage != VK_SHADER_STAGE_FRAGMENT_BIT ||
      stages[1].module != modules.fragment ||
      std::string{stages[1].pName} != "main") {
    return 21;
  }
  return 0;
}

int test_shader_module_structure() {
  const std::string internal =
      read_source("src/renderer/vulkan/vulkan_text_pipeline_internal.hpp");
  const std::string binaries =
      read_source("src/renderer/vulkan/vulkan_text_shader_binaries.cpp");
  const std::string modules =
      read_source("src/renderer/vulkan/vulkan_text_shader_modules.cpp");
  const std::string vertex =
      read_source("src/renderer/vulkan/shaders/text.vert.glsl");
  const std::string fragment =
      read_source("src/renderer/vulkan/shaders/text.frag.glsl");
  if (internal.empty() || binaries.empty() || modules.empty() ||
      vertex.empty() || fragment.empty()) {
    return 30;
  }
  if (!contains(internal, "struct VulkanTextShaderModules") ||
      !contains(binaries, "vulkan_text_vertex_shader_spirv(") ||
      !contains(binaries, "vulkan_text_fragment_shader_spirv(") ||
      !contains(modules, "vulkan_create_text_shader_modules(") ||
      !contains(modules, "vulkan_destroy_text_shader_modules(") ||
      !contains(vertex, "layout(push_constant)") ||
      !contains(fragment, "layout(set = 0, binding = 0)")) {
    return 31;
  }
  return 0;
}

int test_step_468_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 468",
      "embedded SPIR-V",
      "vulkan_text_vertex_shader_spirv",
      "vulkan_create_text_shader_modules",
      "Step 469",
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
  if (const int result = test_embedded_spirv_binaries(); result != 0) {
    return result;
  }
  if (const int result = test_shader_stage_contract(); result != 0) {
    return result;
  }
  if (const int result = test_shader_module_structure(); result != 0) {
    return result;
  }
  return test_step_468_documentation();
}
