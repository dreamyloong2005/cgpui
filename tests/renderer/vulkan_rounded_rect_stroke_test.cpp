#include "vulkan_rounded_rect_geometry_internal.hpp"
#include "vulkan_rounded_rect_radii_internal.hpp"
#include "vulkan_rounded_rect_stroke_internal.hpp"

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

cgpui::RoundedRectDraw stroked_draw(float width = 2.0F) {
  return cgpui::RoundedRectDraw{
      .rect = {.origin = {.x = 10.0F, .y = 20.0F},
               .size = {.width = 40.0F, .height = 20.0F}},
      .color = {.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 1.0F},
      .radius = cgpui::BorderRadii::all(6.0F),
      .border_color = cgpui::Color{.r = 0.9F, .g = 0.1F, .b = 0.2F, .a = 1.0F},
      .border_width = width,
  };
}

int test_stroke_resolution_insets_and_clamps_width() {
  const cgpui::RoundedRectDraw draw = stroked_draw(20.0F);
  const cgpui::BorderRadii outer =
      cgpui::vulkan_resolve_rounded_rect_radii(draw.rect.size, draw.radius).radii;
  const cgpui::VulkanRoundedRectStrokeResolution stroke =
      cgpui::vulkan_resolve_rounded_rect_stroke(draw, outer);
  if (!stroke.enabled || !approximately_equal(stroke.width, 10.0F) ||
      !approximately_equal(stroke.inner_rect.origin.x, 20.0F) ||
      !approximately_equal(stroke.inner_rect.origin.y, 30.0F) ||
      !approximately_equal(stroke.inner_rect.size.width, 20.0F) ||
      !approximately_equal(stroke.inner_rect.size.height, 0.0F) ||
      stroke.color.r != 0.9F) {
    return 10;
  }
  return 0;
}

int test_stroke_geometry_builds_fill_border_and_fringe_rings() {
  const std::array<cgpui::RoundedRectDraw, 1> draws{stroked_draw()};
  const cgpui::VulkanRoundedRectGeometry geometry =
      cgpui::vulkan_build_rounded_rect_geometry(draws, 2);
  if (geometry.vertices.size() != 49 || geometry.indices.size() != 180 ||
      geometry.draws.size() != 1 || geometry.draws[0].vertex_count != 49 ||
      geometry.draws[0].index_count != 180) {
    return 20;
  }
  constexpr std::size_t perimeter_count = 12;
  if (geometry.vertices[1].color !=
          std::array<float, 4>{0.2F, 0.4F, 0.6F, 1.0F} ||
      geometry.vertices[1 + perimeter_count].color !=
          std::array<float, 4>{0.9F, 0.1F, 0.2F, 1.0F} ||
      geometry.vertices[1 + perimeter_count * 2].color !=
          std::array<float, 4>{0.9F, 0.1F, 0.2F, 1.0F} ||
      !approximately_equal(
          geometry.vertices[1 + perimeter_count * 3].coverage, 0.0F)) {
    return 21;
  }
  return 0;
}

int test_fill_only_geometry_stays_compact() {
  cgpui::RoundedRectDraw draw = stroked_draw();
  draw.border_color.reset();
  draw.border_width = 0.0F;
  const std::array<cgpui::RoundedRectDraw, 1> draws{draw};
  const cgpui::VulkanRoundedRectGeometry geometry =
      cgpui::vulkan_build_rounded_rect_geometry(draws, 2);
  return geometry.vertices.size() == 25 && geometry.indices.size() == 108
             ? 0
             : 30;
}

int test_step_481_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 481",
      "VulkanRoundedRectStrokeResolution",
      "inset stroke contour",
      "Step 482",
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
  if (const int result = test_stroke_resolution_insets_and_clamps_width();
      result != 0) {
    return result;
  }
  if (const int result =
          test_stroke_geometry_builds_fill_border_and_fringe_rings();
      result != 0) {
    return result;
  }
  if (const int result = test_fill_only_geometry_stays_compact(); result != 0) {
    return result;
  }
  return test_step_481_documentation();
}
