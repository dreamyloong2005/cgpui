#include "vulkan_composition_opacity_internal.hpp"
#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"
#include "vulkan_rounded_rect_contour_internal.hpp"
#include "vulkan_rounded_rect_geometry_internal.hpp"

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

cgpui::RendererCompositionStackRecord nested_stack(float opacity) {
  return cgpui::RendererCompositionStackRecord{
      .entries = {
          cgpui::PaintMetadata{.opacity = 0.5F},
          cgpui::PaintMetadata{.opacity = opacity},
      },
      .full_depth = 2,
      .current_metadata = cgpui::PaintMetadata{.opacity = opacity},
  };
}

int test_precomposed_opacity_is_applied_once() {
  const cgpui::PaintMetadata metadata{.opacity = 0.25F};
  const cgpui::RendererCompositionStackRecord stack = nested_stack(0.25F);
  if (!approximately_equal(
          cgpui::vulkan_resolve_composed_opacity(metadata, stack), 0.25F)) {
    return 10;
  }
  const cgpui::Color color = cgpui::vulkan_apply_composed_opacity(
      cgpui::Color{.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 0.8F},
      metadata,
      stack);
  return color.r == 0.2F && color.g == 0.4F && color.b == 0.6F &&
                 approximately_equal(color.a, 0.2F)
             ? 0
             : 11;
}

int test_scalar_fallback_and_invalid_values_are_normalized() {
  const cgpui::RendererCompositionStackRecord empty;
  if (!approximately_equal(
          cgpui::vulkan_resolve_composed_opacity(
              cgpui::PaintMetadata{.opacity = 0.4F}, empty),
          0.4F) ||
      cgpui::vulkan_resolve_composed_opacity(
          cgpui::PaintMetadata{.opacity = -1.0F}, empty) != 0.0F ||
      cgpui::vulkan_resolve_composed_opacity(
          cgpui::PaintMetadata{.opacity = 2.0F}, empty) != 1.0F ||
      cgpui::vulkan_resolve_composed_opacity(
          cgpui::PaintMetadata{
              .opacity = std::numeric_limits<float>::quiet_NaN()},
          empty) != 1.0F) {
    return 20;
  }
  return 0;
}

int test_rounded_fill_and_stroke_vertices_apply_opacity() {
  cgpui::RoundedRectDraw draw{
      .rect = {.size = {.width = 40.0F, .height = 24.0F}},
      .color = {.r = 0.2F, .g = 0.3F, .b = 0.4F, .a = 0.8F},
      .radius = cgpui::BorderRadii::all(4.0F),
      .border_color = cgpui::Color{.r = 0.7F, .g = 0.6F, .b = 0.5F, .a = 0.6F},
      .border_width = 2.0F,
  };
  draw.composition_stack = nested_stack(0.25F);
  draw.metadata = cgpui::PaintMetadata{.opacity = 0.25F};
  const std::array<cgpui::RoundedRectDraw, 1> draws{draw};
  const cgpui::VulkanRoundedRectGeometry geometry =
      cgpui::vulkan_build_rounded_rect_geometry(draws, 2);
  const std::size_t perimeter =
      cgpui::vulkan_rounded_rect_perimeter_vertex_count(2);
  if (geometry.vertices.size() <= perimeter + 1 ||
      !approximately_equal(geometry.vertices[0].color[3], 0.2F) ||
      !approximately_equal(
          geometry.vertices[perimeter + 1].color[3], 0.15F)) {
    return 30;
  }
  return 0;
}

int test_production_text_quads_apply_opacity() {
  cgpui::TextDraw text{
      .bounds = {.size = {.width = 32.0F, .height = 20.0F}},
      .color = {.r = 1.0F, .g = 0.8F, .b = 0.6F, .a = 0.8F},
      .font = {.family = "Inter"},
      .content = "a",
      .byte_length = 1,
      .font_size = 16.0F,
      .device_font_size = 16.0F,
  };
  const cgpui::TextShapeRun run =
      cgpui::shape_text(text.content, text.font, text.font_size);
  text.glyphs = cgpui::text_glyph_paint_metadata(run);
  text.composition_stack = nested_stack(0.5F);
  text.metadata = cgpui::PaintMetadata{.opacity = 0.5F};
  const std::array<cgpui::TextDraw, 1> draws{text};
  cgpui::GlyphCache cache;
  const cgpui::VulkanGlyphAtlasDrawData draw_data =
      cgpui::vulkan_plan_glyph_atlas_draw_data(draws, cache);
  return draw_data.quads.size() == 1 &&
                 approximately_equal(draw_data.quads[0].color.a, 0.4F)
             ? 0
             : 40;
}

int test_structure_and_documentation() {
  const std::string opacity_header = read_source(
      "src/renderer/vulkan/vulkan_composition_opacity_internal.hpp");
  const std::string opacity_source = read_source(
      "src/renderer/vulkan/vulkan_composition_opacity.cpp");
  const std::string solid_source =
      read_source("src/renderer/vulkan/vulkan_solid_rect_recording.cpp");
  const std::string rounded_source =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_geometry.cpp");
  const std::string text_source =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_draw_data.cpp");
  if (opacity_header.empty() || opacity_source.empty() || solid_source.empty() ||
      rounded_source.empty() || text_source.empty()) {
    return 50;
  }
  if (!contains(opacity_header, "vulkan_resolve_composed_opacity(") ||
      !contains(opacity_header, "vulkan_apply_composed_opacity(") ||
      !contains(solid_source, "vulkan_apply_composed_opacity(") ||
      !contains(solid_source, "vkCmdClearAttachments") ||
      !contains(rounded_source, "vulkan_apply_composed_opacity(") ||
      !contains(text_source, "vulkan_apply_composed_opacity(")) {
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
      "Phase E Step 484",
      "vulkan_apply_composed_opacity",
      "precomposed opacity",
      "clear writes do not blend",
      "Step 485",
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
  if (const int result = test_precomposed_opacity_is_applied_once();
      result != 0) {
    return result;
  }
  if (const int result = test_scalar_fallback_and_invalid_values_are_normalized();
      result != 0) {
    return result;
  }
  if (const int result = test_rounded_fill_and_stroke_vertices_apply_opacity();
      result != 0) {
    return result;
  }
  if (const int result = test_production_text_quads_apply_opacity();
      result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
