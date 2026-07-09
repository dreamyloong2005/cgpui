#include "vulkan_rounded_rect_geometry_internal.hpp"

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
  return std::abs(left - right) < 0.0001F;
}

int test_rounded_rect_builds_contiguous_geometry() {
  const std::array<cgpui::RoundedRectDraw, 1> draws{
      cgpui::RoundedRectDraw{
          .rect =
              cgpui::Rect{
                  .origin = {.x = 10.0F, .y = 20.0F},
                  .size = {.width = 40.0F, .height = 20.0F},
              },
          .color = {.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 0.8F},
          .radius = cgpui::BorderRadii::all(4.0F),
      },
  };
  const cgpui::VulkanRoundedRectGeometry geometry =
      cgpui::vulkan_build_rounded_rect_geometry(draws, 4);
  if (geometry.draws.size() != 1 || geometry.vertices.size() != 41 ||
      geometry.indices.size() != 180) {
    return 10;
  }
  const cgpui::VulkanRoundedRectDrawRange& range = geometry.draws[0];
  if (range.source_index != 0 || range.first_vertex != 0 ||
      range.vertex_count != 41 || range.first_index != 0 ||
      range.index_count != 180 ||
      !approximately_equal(geometry.vertices[0].position[0], 30.0F) ||
      !approximately_equal(geometry.vertices[0].position[1], 30.0F) ||
      geometry.vertices[0].color !=
          std::array<float, 4>{0.2F, 0.4F, 0.6F, 0.8F}) {
    return 11;
  }
  if (geometry.indices[0] != 0 || geometry.indices[1] != 1 ||
      geometry.indices[2] != 2 || geometry.indices[174] != 20 ||
      geometry.indices[175] != 40 || geometry.indices[176] != 21 ||
      geometry.indices[177] != 20 || geometry.indices[178] != 21 ||
      geometry.indices[179] != 1) {
    return 12;
  }
  return 0;
}

int test_multiple_draws_preserve_offsets_and_skip_empty_rects() {
  const std::array<cgpui::RoundedRectDraw, 3> draws{
      cgpui::RoundedRectDraw{
          .rect = {.size = {.width = 10.0F, .height = 8.0F}},
          .color = {.r = 1.0F, .a = 1.0F},
      },
      cgpui::RoundedRectDraw{
          .rect = {.size = {.width = 0.0F, .height = 8.0F}},
          .color = {.g = 1.0F, .a = 1.0F},
      },
      cgpui::RoundedRectDraw{
          .rect = {.origin = {.x = 20.0F, .y = 10.0F},
                   .size = {.width = 12.0F, .height = 12.0F}},
          .color = {.b = 1.0F, .a = 1.0F},
          .radius = cgpui::BorderRadii::all(3.0F),
      },
  };
  const cgpui::VulkanRoundedRectGeometry geometry =
      cgpui::vulkan_build_rounded_rect_geometry(draws, 2);
  if (geometry.draws.size() != 2 || geometry.draws[0].source_index != 0 ||
      geometry.draws[1].source_index != 2 ||
      geometry.draws[1].first_vertex != geometry.draws[0].vertex_count ||
      geometry.draws[1].first_index != geometry.draws[0].index_count) {
    return 20;
  }
  for (std::uint32_t index : geometry.indices) {
    if (index >= geometry.vertices.size()) {
      return 21;
    }
  }
  return 0;
}

int test_rounded_rect_geometry_structure() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_geometry_internal.hpp");
  const std::string source =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_geometry.cpp");
  const std::string contour =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_contour.cpp");
  const std::string command =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  if (header.empty() || source.empty() || contour.empty() || command.empty()) {
    return 30;
  }
  if (!contains(header, "struct VulkanRoundedRectGeometry") ||
      !contains(header, "struct VulkanRoundedRectDrawRange") ||
      !contains(source, "vulkan_build_rounded_rect_geometry(") ||
      !contains(source, "vulkan_append_rounded_rect_contour(") ||
      !contains(contour, "append_corner_arc(") ||
      !contains(source, "geometry.vertices.reserve(total_vertex_count)") ||
      contains(source, "append_corner_arc(") ||
      contains(command, "append_corner_arc(")) {
    return 31;
  }
  return 0;
}

int test_step_475_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 475",
      "VulkanRoundedRectGeometry",
      "contiguous vertex/index buffers",
      "Step 476",
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
  if (const int result = test_rounded_rect_builds_contiguous_geometry();
      result != 0) {
    return result;
  }
  if (const int result =
          test_multiple_draws_preserve_offsets_and_skip_empty_rects();
      result != 0) {
    return result;
  }
  if (const int result = test_rounded_rect_geometry_structure(); result != 0) {
    return result;
  }
  return test_step_475_documentation();
}
