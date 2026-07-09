#include "vulkan_rounded_rect_geometry_internal.hpp"
#include "vulkan_rounded_rect_radii_internal.hpp"

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

int test_oversized_radii_share_one_normalization_scale() {
  const cgpui::VulkanRoundedRectRadiiResolution resolution =
      cgpui::vulkan_resolve_rounded_rect_radii(
          cgpui::Size{.width = 40.0F, .height = 20.0F},
          cgpui::BorderRadii::all(30.0F));
  if (!resolution.normalized ||
      !approximately_equal(resolution.scale, 1.0F / 3.0F) ||
      !approximately_equal(resolution.radii.top_left, 10.0F) ||
      !approximately_equal(resolution.radii.top_right, 10.0F) ||
      !approximately_equal(resolution.radii.bottom_right, 10.0F) ||
      !approximately_equal(resolution.radii.bottom_left, 10.0F)) {
    return 10;
  }
  return 0;
}

int test_negative_and_asymmetric_radii_are_bounded() {
  const cgpui::VulkanRoundedRectRadiiResolution resolution =
      cgpui::vulkan_resolve_rounded_rect_radii(
          cgpui::Size{.width = 100.0F, .height = 50.0F},
          cgpui::BorderRadii::corners(-4.0F, 80.0F, 40.0F, 30.0F));
  if (!resolution.normalized || resolution.radii.top_left != 0.0F ||
      resolution.radii.top_right + resolution.radii.bottom_right > 50.0F ||
      resolution.radii.bottom_left + resolution.radii.bottom_right > 100.0F) {
    return 20;
  }
  const auto unchanged = cgpui::vulkan_resolve_rounded_rect_radii(
      cgpui::Size{.width = 100.0F, .height = 50.0F},
      cgpui::BorderRadii::all(8.0F));
  return !unchanged.normalized && approximately_equal(unchanged.scale, 1.0F)
             ? 0
             : 21;
}

int test_geometry_uses_normalized_inner_contour() {
  const std::array<cgpui::RoundedRectDraw, 1> draws{
      cgpui::RoundedRectDraw{
          .rect = {.origin = {.x = 10.0F, .y = 20.0F},
                   .size = {.width = 40.0F, .height = 20.0F}},
          .color = {.r = 1.0F, .a = 1.0F},
          .radius = cgpui::BorderRadii::all(30.0F),
      },
  };
  const cgpui::VulkanRoundedRectGeometry geometry =
      cgpui::vulkan_build_rounded_rect_geometry(draws, 2);
  constexpr std::size_t perimeter_count = 12;
  for (std::size_t index = 1; index <= perimeter_count; ++index) {
    const auto& position = geometry.vertices[index].position;
    if (position[0] < 10.0F || position[0] > 50.0F ||
        position[1] < 20.0F || position[1] > 40.0F) {
      return 30;
    }
  }
  return 0;
}

int test_step_480_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 480",
      "VulkanRoundedRectRadiiResolution",
      "adjacent corner sums",
      "Step 481",
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
  if (const int result = test_oversized_radii_share_one_normalization_scale();
      result != 0) {
    return result;
  }
  if (const int result = test_negative_and_asymmetric_radii_are_bounded();
      result != 0) {
    return result;
  }
  if (const int result = test_geometry_uses_normalized_inner_contour();
      result != 0) {
    return result;
  }
  return test_step_480_documentation();
}
