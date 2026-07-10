#include "vulkan_composition_transform_internal.hpp"
#include "vulkan_rounded_rect_geometry_internal.hpp"
#include "vulkan_solid_rect_geometry_internal.hpp"
#include "vulkan_text_vertex_buffer_internal.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
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

bool same_point(cgpui::Point point, float x, float y) {
  return approximately_equal(point.x, x) && approximately_equal(point.y, y);
}

cgpui::RendererCompositionStackRecord transform_stack(
    cgpui::AffineTransform transform) {
  return cgpui::RendererCompositionStackRecord{
      .entries = {
          cgpui::PaintMetadata{
              .transform = cgpui::AffineTransform::translation(10.0F, 20.0F)},
          cgpui::PaintMetadata{.transform = transform},
      },
      .full_depth = 2,
      .current_metadata = cgpui::PaintMetadata{.transform = transform},
  };
}

int test_precomposed_transform_is_applied_once() {
  const cgpui::AffineTransform transform = cgpui::compose(
      cgpui::AffineTransform::translation(10.0F, 20.0F),
      cgpui::AffineTransform::scale(2.0F, 3.0F));
  const cgpui::PaintMetadata metadata{.transform = transform};
  const cgpui::RendererCompositionStackRecord stack =
      transform_stack(transform);
  const cgpui::Point point = cgpui::vulkan_apply_composed_transform(
      cgpui::Point{.x = 1.0F, .y = 2.0F}, metadata, stack);
  return same_point(point, 12.0F, 26.0F) ? 0 : 10;
}

int test_skew_and_invalid_transform_policy() {
  const cgpui::AffineTransform skewed{
      .scale_x = 1.0F,
      .skew_y = 0.5F,
      .skew_x = 2.0F,
      .scale_y = 1.0F,
      .translate_x = 3.0F,
      .translate_y = 4.0F,
  };
  if (!same_point(
          cgpui::vulkan_transform_point(
              cgpui::Point{.x = 2.0F, .y = 5.0F}, skewed),
          15.0F,
          10.0F)) {
    return 20;
  }
  cgpui::PaintMetadata invalid;
  invalid.transform.translate_x = std::numeric_limits<float>::quiet_NaN();
  return same_point(
             cgpui::vulkan_apply_composed_transform(
                 cgpui::Point{.x = 2.0F, .y = 5.0F},
                 invalid,
                 cgpui::RendererCompositionStackRecord{}),
             2.0F,
             5.0F)
             ? 0
             : 21;
}

int test_solid_and_rounded_vertices_transform() {
  cgpui::SolidRect solid{
      .rect = {.origin = {.x = 1.0F, .y = 2.0F},
               .size = {.width = 3.0F, .height = 4.0F}},
      .color = {.r = 1.0F, .a = 1.0F},
  };
  solid.metadata.transform = cgpui::AffineTransform::translation(5.0F, 7.0F);
  const std::array<cgpui::SolidRect, 1> solids{solid};
  const cgpui::VulkanRoundedRectGeometry solid_geometry =
      cgpui::vulkan_build_solid_rect_geometry(solids);
  if (solid_geometry.vertices.size() != 4 ||
      solid_geometry.vertices[0].position !=
          std::array<float, 2>{6.0F, 9.0F} ||
      solid_geometry.vertices[2].position !=
          std::array<float, 2>{9.0F, 13.0F}) {
    return 30;
  }

  cgpui::RoundedRectDraw rounded{
      .rect = {.origin = {.x = 10.0F, .y = 20.0F},
               .size = {.width = 8.0F, .height = 4.0F}},
      .color = {.g = 1.0F, .a = 1.0F},
      .radius = cgpui::BorderRadii::all(2.0F),
  };
  rounded.metadata.transform = cgpui::AffineTransform::scale(2.0F, 3.0F);
  const std::array<cgpui::RoundedRectDraw, 1> rounded_draws{rounded};
  const cgpui::VulkanRoundedRectGeometry rounded_geometry =
      cgpui::vulkan_build_rounded_rect_geometry(rounded_draws, 2);
  return rounded_geometry.vertices.size() > 1 &&
                 rounded_geometry.vertices[0].position ==
                     std::array<float, 2>{28.0F, 66.0F}
             ? 0
             : 31;
}

int test_text_quad_corners_transform_after_positioning() {
  cgpui::TexturedGlyphQuad quad{
      .device_bounds = {.origin = {.x = 1.0F, .y = 2.0F},
                        .size = {.width = 3.0F, .height = 4.0F}},
      .atlas_uv_bounds = {.origin = {.x = 0.1F, .y = 0.2F},
                          .size = {.width = 0.3F, .height = 0.4F}},
      .color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
  };
  quad.metadata.transform = cgpui::AffineTransform{
      .scale_x = 2.0F,
      .scale_y = 3.0F,
      .translate_x = 5.0F,
      .translate_y = 7.0F,
  };
  const std::array<cgpui::TexturedGlyphQuad, 1> quads{quad};
  const std::vector<cgpui::VulkanTextVertex> vertices =
      cgpui::vulkan_build_text_vertices(quads);
  return vertices.size() == 6 &&
                 vertices[0].position == std::array<float, 2>{7.0F, 13.0F} &&
                 vertices[2].position == std::array<float, 2>{13.0F, 25.0F}
             ? 0
             : 40;
}

int test_structure_and_documentation() {
  const std::string transform_header = read_source(
      "src/renderer/vulkan/vulkan_composition_transform_internal.hpp");
  const std::string transform_source = read_source(
      "src/renderer/vulkan/vulkan_composition_transform.cpp");
  const std::string solid =
      read_source("src/renderer/vulkan/vulkan_solid_rect_geometry.cpp");
  const std::string rounded =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_geometry.cpp");
  const std::string text =
      read_source("src/renderer/vulkan/vulkan_text_vertex_buffer.cpp");
  if (transform_header.empty() || transform_source.empty() || solid.empty() ||
      rounded.empty() || text.empty()) {
    return 50;
  }
  if (!contains(transform_header, "vulkan_resolve_composed_transform(") ||
      !contains(transform_header, "vulkan_transform_point(") ||
      !contains(transform_header, "vulkan_apply_composed_transform(") ||
      !contains(solid, "vulkan_apply_composed_transform(") ||
      !contains(rounded, "vulkan_apply_composed_transform(") ||
      !contains(text, "vulkan_apply_composed_transform(")) {
    return 51;
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
      "Phase E Step 486",
      "vulkan_apply_composed_transform",
      "precomposed affine transform",
      "Step 487",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 60;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_precomposed_transform_is_applied_once();
      result != 0) {
    return result;
  }
  if (const int result = test_skew_and_invalid_transform_policy();
      result != 0) {
    return result;
  }
  if (const int result = test_solid_and_rounded_vertices_transform();
      result != 0) {
    return result;
  }
  if (const int result = test_text_quad_corners_transform_after_positioning();
      result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
