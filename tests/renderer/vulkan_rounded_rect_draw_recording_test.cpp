#include "vulkan_rounded_rect_draw_recording_internal.hpp"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
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

cgpui::VulkanRoundedRectPipelineResources pipeline_resources() {
  return cgpui::VulkanRoundedRectPipelineResources{
      .layout = fake_handle<VkPipelineLayout>(1),
      .pipeline = fake_handle<VkPipeline>(2),
  };
}

cgpui::VulkanRoundedRectBufferResources buffer_resources() {
  cgpui::VulkanRoundedRectBufferResources resources{
      .vertex_buffer = fake_handle<VkBuffer>(3),
      .vertex_memory = fake_handle<VkDeviceMemory>(4),
      .index_buffer = fake_handle<VkBuffer>(5),
      .index_memory = fake_handle<VkDeviceMemory>(6),
      .vertex_count = 10,
      .index_count = 24,
      .vertex_byte_size =
          10 * sizeof(cgpui::VulkanRoundedRectVertex),
      .index_byte_size = 24 * sizeof(std::uint32_t),
  };
  resources.draws = {
      cgpui::VulkanRoundedRectDrawRange{
          .source_index = 0,
          .first_vertex = 0,
          .vertex_count = 5,
          .first_index = 0,
          .index_count = 12,
      },
      cgpui::VulkanRoundedRectDrawRange{
          .source_index = 1,
          .first_vertex = 5,
          .vertex_count = 5,
          .first_index = 12,
          .index_count = 12,
      },
  };
  return resources;
}

int test_uploaded_draw_ranges_are_validated() {
  const auto result = cgpui::vulkan_validate_rounded_rect_draw_resources(
      pipeline_resources(), buffer_resources());
  if (!result) {
    return 10;
  }
  const auto empty = cgpui::vulkan_validate_rounded_rect_draw_resources(
      cgpui::VulkanRoundedRectPipelineResources{},
      cgpui::VulkanRoundedRectBufferResources{});
  return empty ? 0 : 11;
}

int test_invalid_draw_ranges_are_rejected() {
  auto buffers = buffer_resources();
  buffers.draws[1].index_count = 13;
  if (cgpui::vulkan_validate_rounded_rect_draw_resources(
          pipeline_resources(), buffers)) {
    return 20;
  }
  buffers = buffer_resources();
  buffers.draws[0].first_index =
      static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()) + 1;
  if (cgpui::vulkan_validate_rounded_rect_draw_resources(
          pipeline_resources(), buffers)) {
    return 21;
  }
  buffers = buffer_resources();
  if (cgpui::vulkan_validate_rounded_rect_draw_resources(
          cgpui::VulkanRoundedRectPipelineResources{}, buffers)) {
    return 22;
  }
  return 0;
}

int test_rounded_rect_draw_recording_structure() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_draw_recording_internal.hpp");
  const std::string source = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_draw_recording.cpp");
  const std::string command =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string ordered =
      read_source("src/renderer/vulkan/vulkan_frame_draw_recording.cpp");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  if (header.empty() || source.empty() || command.empty() || ordered.empty() ||
      presentation.empty()) {
    return 30;
  }
  const char* required[] = {
      "vkCmdBindPipeline",
      "vkCmdSetViewport",
      "vkCmdSetScissor",
      "vkCmdBindVertexBuffers",
      "vkCmdBindIndexBuffer",
      "vkCmdPushConstants",
      "vkCmdDrawIndexed",
  };
  for (const char* value : required) {
    if (!contains(source, value)) {
      return 31;
    }
  }
  if (!contains(command, "vulkan_record_frame_draws(") ||
      contains(command, "vulkan_record_rounded_rect_draws(") ||
      !contains(ordered, "vulkan_record_rounded_rect_draw(") ||
      contains(command, "vkCmdDrawIndexed") ||
      !contains(presentation, "rounded_rect_pipeline_resources_") ||
      !contains(presentation, "rounded_rect_buffers_")) {
    return 32;
  }
  return 0;
}

int test_step_478_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 478",
      "vulkan_record_rounded_rect_draws",
      "vkCmdDrawIndexed",
      "Step 479",
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
  if (const int result = test_uploaded_draw_ranges_are_validated();
      result != 0) {
    return result;
  }
  if (const int result = test_invalid_draw_ranges_are_rejected(); result != 0) {
    return result;
  }
  if (const int result = test_rounded_rect_draw_recording_structure();
      result != 0) {
    return result;
  }
  return test_step_478_documentation();
}
