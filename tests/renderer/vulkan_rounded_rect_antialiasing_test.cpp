#include "vulkan_rounded_rect_antialiasing_internal.hpp"
#include "vulkan_rounded_rect_geometry_internal.hpp"
#include "vulkan_rounded_rect_pipeline_internal.hpp"

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

int test_default_policy_uses_one_pixel_coverage_fringe() {
  const cgpui::VulkanRoundedRectAntialiasingPolicy policy =
      cgpui::vulkan_default_rounded_rect_antialiasing_policy();
  if (policy.mode !=
          cgpui::VulkanRoundedRectAntialiasingMode::coverage_fringe ||
      !approximately_equal(policy.fringe_width, 1.0F) ||
      !approximately_equal(cgpui::vulkan_rounded_rect_coverage(-1.0F), 0.0F) ||
      !approximately_equal(cgpui::vulkan_rounded_rect_coverage(0.4F), 0.4F) ||
      !approximately_equal(cgpui::vulkan_rounded_rect_coverage(2.0F), 1.0F)) {
    return 10;
  }
  return 0;
}

int test_geometry_builds_inner_and_outer_coverage_rings() {
  const std::array<cgpui::RoundedRectDraw, 1> draws{
      cgpui::RoundedRectDraw{
          .rect = {.origin = {.x = 10.0F, .y = 20.0F},
                   .size = {.width = 40.0F, .height = 20.0F}},
          .color = {.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 0.8F},
          .radius = cgpui::BorderRadii::all(4.0F),
      },
  };
  const cgpui::VulkanRoundedRectGeometry geometry =
      cgpui::vulkan_build_rounded_rect_geometry(draws, 2);
  if (geometry.draws.size() != 1 || geometry.vertices.size() != 25 ||
      geometry.indices.size() != 108 ||
      !approximately_equal(geometry.vertices[0].coverage, 1.0F)) {
    return 20;
  }
  constexpr std::size_t inner_vertex_count = 12;
  for (std::size_t index = 1; index <= inner_vertex_count; ++index) {
    if (!approximately_equal(geometry.vertices[index].coverage, 1.0F)) {
      return 21;
    }
  }
  for (std::size_t index = inner_vertex_count + 1;
       index < geometry.vertices.size(); ++index) {
    if (!approximately_equal(geometry.vertices[index].coverage, 0.0F)) {
      return 22;
    }
  }
  return 0;
}

int test_pipeline_and_shader_carry_coverage() {
  const auto attributes =
      cgpui::vulkan_rounded_rect_vertex_attribute_descriptions();
  if (attributes.size() != 3 || attributes[2].location != 2 ||
      attributes[2].format != VK_FORMAT_R32_SFLOAT) {
    return 30;
  }
  const std::string vertex = read_source(
      "src/renderer/vulkan/shaders/rounded_rect.vert.glsl");
  const std::string fragment = read_source(
      "src/renderer/vulkan/shaders/rounded_rect.frag.glsl");
  if (!contains(vertex, "in_coverage") ||
      !contains(vertex, "out_coverage = in_coverage") ||
      !contains(fragment, "in_coverage") ||
      !contains(fragment, "in_color.a * clamp(in_coverage")) {
    return 31;
  }
  return 0;
}

int test_step_479_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 479",
      "VulkanRoundedRectAntialiasingPolicy",
      "coverage fringe",
      "Step 480",
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
  if (const int result = test_default_policy_uses_one_pixel_coverage_fringe();
      result != 0) {
    return result;
  }
  if (const int result = test_geometry_builds_inner_and_outer_coverage_rings();
      result != 0) {
    return result;
  }
  if (const int result = test_pipeline_and_shader_carry_coverage();
      result != 0) {
    return result;
  }
  return test_step_479_documentation();
}
