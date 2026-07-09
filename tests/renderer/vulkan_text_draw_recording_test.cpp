#include "vulkan_text_draw_recording_internal.hpp"

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

cgpui::VulkanTextVertexBufferResources vertex_buffer(
    std::size_t vertex_count) {
  return cgpui::VulkanTextVertexBufferResources{
      .buffer = fake_handle<VkBuffer>(1),
      .memory = fake_handle<VkDeviceMemory>(2),
      .vertex_count = vertex_count,
      .byte_size = vertex_count * sizeof(cgpui::VulkanTextVertex),
  };
}

cgpui::VulkanTextPipelineResources pipeline_resources() {
  return cgpui::VulkanTextPipelineResources{
      .layout = fake_handle<VkPipelineLayout>(6),
      .pipeline = fake_handle<VkPipeline>(7),
  };
}

int test_page_runs_plan_vertex_draw_ranges() {
  const std::array<cgpui::VulkanGlyphAtlasDrawBinding, 2> bindings{
      cgpui::VulkanGlyphAtlasDrawBinding{
          .page_index = 0,
          .first_quad_index = 0,
          .glyph_quad_count = 2,
          .descriptor_set = fake_handle<VkDescriptorSet>(3),
      },
      cgpui::VulkanGlyphAtlasDrawBinding{
          .page_index = 1,
          .first_quad_index = 2,
          .glyph_quad_count = 2,
          .descriptor_set = fake_handle<VkDescriptorSet>(4),
      },
  };
  const auto commands = cgpui::vulkan_plan_text_draw_commands(
      bindings,
      pipeline_resources(),
      vertex_buffer(24));
  if (!commands || commands->size() != 2 ||
      (*commands)[0].descriptor_set != bindings[0].descriptor_set ||
      (*commands)[0].first_vertex != 0 ||
      (*commands)[0].vertex_count != 12 ||
      (*commands)[1].descriptor_set != bindings[1].descriptor_set ||
      (*commands)[1].first_vertex != 12 ||
      (*commands)[1].vertex_count != 12) {
    return 10;
  }
  return 0;
}

int test_invalid_draw_ranges_are_rejected() {
  cgpui::VulkanGlyphAtlasDrawBinding binding{
      .first_quad_index = 1,
      .glyph_quad_count = 1,
      .descriptor_set = fake_handle<VkDescriptorSet>(5),
  };
  const std::array<cgpui::VulkanGlyphAtlasDrawBinding, 1> bindings{binding};
  if (cgpui::vulkan_plan_text_draw_commands(
          bindings, pipeline_resources(), vertex_buffer(11))) {
    return 20;
  }
  binding.descriptor_set = VK_NULL_HANDLE;
  const std::array<cgpui::VulkanGlyphAtlasDrawBinding, 1> missing{binding};
  if (cgpui::vulkan_plan_text_draw_commands(
          missing, pipeline_resources(), vertex_buffer(12))) {
    return 21;
  }
  const std::span<const cgpui::VulkanGlyphAtlasDrawBinding> empty;
  const cgpui::VulkanTextVertexBufferResources no_vertices;
  const auto no_commands =
      cgpui::vulkan_plan_text_draw_commands(
          empty,
          cgpui::VulkanTextPipelineResources{},
          no_vertices);
  return no_commands && no_commands->empty() ? 0 : 22;
}

int test_text_draw_recording_structure() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_text_draw_recording_internal.hpp");
  const std::string source =
      read_source("src/renderer/vulkan/vulkan_text_draw_recording.cpp");
  const std::string command =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  if (header.empty() || source.empty() || command.empty() ||
      presentation.empty()) {
    return 30;
  }
  const char* required[] = {
      "vkCmdBindPipeline",
      "vkCmdSetViewport",
      "vkCmdSetScissor",
      "vkCmdBindVertexBuffers",
      "vkCmdPushConstants",
      "vkCmdBindDescriptorSets",
      "vkCmdDraw",
  };
  for (const char* value : required) {
    if (!contains(source, value)) {
      return 31;
    }
  }
  if (!contains(command, "vulkan_record_text_draws(") ||
      contains(command, "vkCmdDraw") ||
      !contains(presentation, "text_pipeline_resources_") ||
      !contains(presentation, "text_vertex_buffer_")) {
    return 32;
  }
  return 0;
}

int test_step_471_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 471",
      "vulkan_record_text_draws",
      "vkCmdDraw",
      "Step 472",
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
  if (const int result = test_page_runs_plan_vertex_draw_ranges(); result != 0) {
    return result;
  }
  if (const int result = test_invalid_draw_ranges_are_rejected(); result != 0) {
    return result;
  }
  if (const int result = test_text_draw_recording_structure(); result != 0) {
    return result;
  }
  return test_step_471_documentation();
}
