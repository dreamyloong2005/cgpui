#include "vulkan_frame_pipeline_switch_internal.hpp"

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

int test_shared_rounded_rect_pipeline_is_coalesced() {
  cgpui::VulkanFramePipelineSwitchState state;
  const auto solid = cgpui::vulkan_plan_frame_pipeline_switch(
      state, cgpui::VulkanFrameDrawResourceKind::solid_rect);
  const auto rounded = cgpui::vulkan_plan_frame_pipeline_switch(
      state, cgpui::VulkanFrameDrawResourceKind::rounded_rect);
  const auto solid_again = cgpui::vulkan_plan_frame_pipeline_switch(
      state, cgpui::VulkanFrameDrawResourceKind::solid_rect);

  if (solid.pipeline_kind !=
          cgpui::VulkanFramePipelineKind::rounded_rect ||
      !solid.bind_pipeline || !solid.bind_geometry) {
    return 10;
  }
  if (rounded.pipeline_kind !=
          cgpui::VulkanFramePipelineKind::rounded_rect ||
      rounded.bind_pipeline || !rounded.bind_geometry) {
    return 11;
  }
  return !solid_again.bind_pipeline && solid_again.bind_geometry ? 0 : 12;
}

int test_identical_adjacent_resources_need_no_state_bind() {
  cgpui::VulkanFramePipelineSwitchState state;
  (void)cgpui::vulkan_plan_frame_pipeline_switch(
      state, cgpui::VulkanFrameDrawResourceKind::text);
  const auto text_again = cgpui::vulkan_plan_frame_pipeline_switch(
      state, cgpui::VulkanFrameDrawResourceKind::text);
  return text_again.pipeline_kind == cgpui::VulkanFramePipelineKind::text &&
                 !text_again.bind_pipeline && !text_again.bind_geometry
             ? 0
             : 20;
}

int test_distinct_pipelines_switch_without_reordering() {
  cgpui::VulkanFramePipelineSwitchState state;
  const auto solid = cgpui::vulkan_plan_frame_pipeline_switch(
      state, cgpui::VulkanFrameDrawResourceKind::solid_rect);
  const auto text = cgpui::vulkan_plan_frame_pipeline_switch(
      state, cgpui::VulkanFrameDrawResourceKind::text);
  const auto image = cgpui::vulkan_plan_frame_pipeline_switch(
      state, cgpui::VulkanFrameDrawResourceKind::image);

  if (solid.resource_kind !=
          cgpui::VulkanFrameDrawResourceKind::solid_rect ||
      text.resource_kind != cgpui::VulkanFrameDrawResourceKind::text ||
      image.resource_kind != cgpui::VulkanFrameDrawResourceKind::image) {
    return 30;
  }
  return text.bind_pipeline && text.bind_geometry &&
                 image.bind_pipeline && image.bind_geometry
             ? 0
             : 31;
}

int test_structure_and_documentation() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_frame_pipeline_switch_internal.hpp");
  const std::string source = read_source(
      "src/renderer/vulkan/vulkan_frame_pipeline_switch.cpp");
  const std::string recording = read_source(
      "src/renderer/vulkan/vulkan_frame_draw_recording.cpp");
  const std::string rounded = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_draw_recording.cpp");
  const std::string structure = read_source(
      "tests/architecture/renderer_source_structure_test.cpp");
  if (header.empty() || source.empty() || recording.empty() ||
      rounded.empty() || structure.empty()) {
    return 40;
  }
  if (!contains(header, "struct VulkanFramePipelineSwitchState") ||
      !contains(source, "vulkan_plan_frame_pipeline_switch(") ||
      !contains(recording, "VulkanFramePipelineSwitchState") ||
      !contains(recording, ".bind_pipeline") ||
      !contains(rounded, "if (bind_pipeline)") ||
      contains(source, "vkCmdBindPipeline") ||
      !contains(structure, "vulkan_frame_pipeline_switch.cpp")) {
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
      "Phase E Step 509",
      "pipeline-switch batching",
      "shared rounded-rectangle pipeline",
      "authored draw order preserved",
      "Step 510 resource barriers",
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
  if (const int result = test_shared_rounded_rect_pipeline_is_coalesced();
      result != 0) {
    return result;
  }
  if (const int result = test_identical_adjacent_resources_need_no_state_bind();
      result != 0) {
    return result;
  }
  if (const int result = test_distinct_pipelines_switch_without_reordering();
      result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
