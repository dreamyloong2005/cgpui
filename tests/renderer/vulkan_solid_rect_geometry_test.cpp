#include "vulkan_solid_rect_geometry_internal.hpp"

#include <array>
#include <cmath>
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

bool approximately_equal(float left, float right) {
  return std::fabs(left - right) < 0.0001F;
}

int test_solid_rects_build_compact_blended_geometry() {
  cgpui::SolidRect first{
      .rect = {.origin = {.x = 2.0F, .y = 3.0F},
               .size = {.width = 10.0F, .height = 6.0F}},
      .color = {.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 0.8F},
  };
  first.metadata.opacity = 0.5F;
  cgpui::SolidRect clipped{
      .rect = {.origin = {.x = 20.0F, .y = 10.0F},
               .size = {.width = 8.0F, .height = 4.0F}},
      .color = {.r = 0.7F, .g = 0.3F, .b = 0.1F, .a = 1.0F},
      .clip_rect = cgpui::Rect{
          .origin = {.x = 21.0F, .y = 11.0F},
          .size = {.width = 5.0F, .height = 2.0F},
      },
  };
  const std::array<cgpui::SolidRect, 3> rects{
      first,
      cgpui::SolidRect{.rect = {.size = {.width = 0.0F, .height = 4.0F}}},
      clipped,
  };
  const cgpui::VulkanRoundedRectGeometry geometry =
      cgpui::vulkan_build_solid_rect_geometry(rects);
  if (geometry.vertices.size() != 8 || geometry.indices.size() != 12 ||
      geometry.draws.size() != 2 || geometry.draws[0].source_index != 0 ||
      geometry.draws[1].source_index != 2) {
    return 10;
  }
  if (geometry.vertices[0].position != std::array<float, 2>{2.0F, 3.0F} ||
      geometry.vertices[1].position != std::array<float, 2>{12.0F, 3.0F} ||
      geometry.vertices[2].position != std::array<float, 2>{12.0F, 9.0F} ||
      geometry.vertices[3].position != std::array<float, 2>{2.0F, 9.0F} ||
      !approximately_equal(geometry.vertices[0].color[3], 0.4F) ||
      geometry.indices != std::vector<std::uint32_t>{
          0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7}) {
    return 11;
  }
  return geometry.draws[1].clip_rect.has_value() &&
                 geometry.draws[1].clip_rect->origin.x == 21.0F &&
                 geometry.draws[1].clip_rect->size.height == 2.0F
             ? 0
             : 12;
}

int test_solid_geometry_owns_the_promoted_frame_path() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_solid_rect_geometry_internal.hpp");
  const std::string geometry =
      read_source("src/renderer/vulkan/vulkan_solid_rect_geometry.cpp");
  const std::string buffers =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_buffers.cpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_solid_rect_frame.cpp");
  const std::string state =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  const std::string command =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string old_recording =
      read_source("src/renderer/vulkan/vulkan_solid_rect_recording.cpp");
  const std::string pipeline = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_pipeline_state.cpp");
  if (header.empty() || geometry.empty() || buffers.empty() || frame.empty() ||
      state.empty() || command.empty() || pipeline.empty()) {
    return 20;
  }
  if (!contains(header, "vulkan_build_solid_rect_geometry(") ||
      !contains(geometry, "vulkan_apply_composed_opacity(") ||
      !contains(geometry, "vulkan_resolve_effective_clip_rect(") ||
      !contains(buffers, "vulkan_upload_solid_rect_buffers(") ||
      !contains(frame, "prepare_solid_rect_frame(") ||
      !contains(state, "solid_rect_buffers_") ||
      !contains(command, "solid_rect_buffers") ||
      contains(command, "vulkan_record_solid_rects(") ||
      contains(command, "vkCmdClearAttachments") ||
      !old_recording.empty() ||
      !contains(pipeline, "VK_BLEND_FACTOR_SRC_ALPHA")) {
    return 21;
  }
  return 0;
}

int test_step_485_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 485",
      "vulkan_build_solid_rect_geometry",
      "blend-capable solid geometry",
      "Step 486",
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
  if (const int result = test_solid_rects_build_compact_blended_geometry();
      result != 0) {
    return result;
  }
  if (const int result = test_solid_geometry_owns_the_promoted_frame_path();
      result != 0) {
    return result;
  }
  return test_step_485_documentation();
}
