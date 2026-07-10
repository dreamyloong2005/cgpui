#include "vulkan_image_color_internal.hpp"
#include "vulkan_image_pipeline_internal.hpp"
#include "vulkan_image_vertex_buffer_internal.hpp"

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

bool approximately_equal(float lhs, float rhs) {
  return std::abs(lhs - rhs) < 0.0001F;
}

bool same_color(const std::array<float, 4>& actual, cgpui::Color expected) {
  return approximately_equal(actual[0], expected.r) &&
         approximately_equal(actual[1], expected.g) &&
         approximately_equal(actual[2], expected.b) &&
         approximately_equal(actual[3], expected.a);
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

int test_default_tint_uses_white_and_precomposed_opacity() {
  cgpui::ImageDraw draw{
      .bounds = {.size = {.width = 24.0F, .height = 12.0F}},
      .metadata = cgpui::PaintMetadata{.opacity = 0.25F},
  };
  draw.composition_stack = nested_stack(0.25F);
  const cgpui::Color tint = cgpui::vulkan_resolve_image_tint(draw);
  if (!approximately_equal(tint.r, 1.0F) ||
      !approximately_equal(tint.g, 1.0F) ||
      !approximately_equal(tint.b, 1.0F) ||
      !approximately_equal(tint.a, 0.25F)) {
    return 10;
  }
  const std::array<cgpui::ImageDraw, 1> draws{draw};
  const auto vertices = cgpui::vulkan_build_image_vertices(draws);
  return vertices.size() == cgpui::vulkan_image_vertices_per_quad &&
                 same_color(vertices.front().color, tint) &&
                 same_color(vertices.back().color, tint)
             ? 0
             : 11;
}

int test_authored_tint_multiplies_sample_and_opacity_once() {
  cgpui::ImageDraw draw{
      .bounds = {.size = {.width = 16.0F, .height = 16.0F}},
      .tint = cgpui::Color{.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 0.8F},
      .metadata = cgpui::PaintMetadata{.opacity = 0.5F},
  };
  draw.composition_stack = nested_stack(0.5F);
  const cgpui::Color tint = cgpui::vulkan_resolve_image_tint(draw);
  return approximately_equal(tint.r, 0.2F) &&
                 approximately_equal(tint.g, 0.4F) &&
                 approximately_equal(tint.b, 0.6F) &&
                 approximately_equal(tint.a, 0.4F)
             ? 0
             : 20;
}

int test_vertex_and_shader_color_contract() {
  const auto attributes =
      cgpui::vulkan_image_vertex_attribute_descriptions();
  if (attributes.size() != 3 ||
      attributes[2].format != VK_FORMAT_R32G32B32A32_SFLOAT) {
    return 30;
  }
  const auto vertex_spirv = cgpui::vulkan_image_vertex_shader_spirv();
  const auto fragment_spirv = cgpui::vulkan_image_fragment_shader_spirv();
  return !vertex_spirv.empty() && !fragment_spirv.empty() &&
                 vertex_spirv.front() == 0x07230203U &&
                 fragment_spirv.front() == 0x07230203U
             ? 0
             : 31;
}

int test_structure_and_documentation() {
  const std::string color_header = read_source(
      "src/renderer/vulkan/vulkan_image_color_internal.hpp");
  const std::string color_source =
      read_source("src/renderer/vulkan/vulkan_image_color.cpp");
  const std::string vertex_source =
      read_source("src/renderer/vulkan/vulkan_image_vertex_buffer.cpp");
  const std::string vertex_shader =
      read_source("src/renderer/vulkan/shaders/image.vert.glsl");
  const std::string fragment_shader =
      read_source("src/renderer/vulkan/shaders/image.frag.glsl");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (color_header.empty() || color_source.empty() || vertex_source.empty() ||
      vertex_shader.empty() || fragment_shader.empty() || structure.empty()) {
    return 40;
  }
  if (!contains(color_header, "vulkan_resolve_image_tint(") ||
      !contains(color_source, "vulkan_apply_composed_opacity(") ||
      !contains(vertex_source, "vulkan_resolve_image_tint(") ||
      !contains(vertex_shader, "out_color") ||
      !contains(fragment_shader, "texture(image_texture, in_image_uv) *") ||
      !contains(structure, "vulkan_image_color.cpp")) {
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
      "Phase E Step 495",
      "multiplicative image tint",
      "composition opacity",
      "single application",
      "Step 496 image cache lifetime",
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
  if (const int result =
          test_default_tint_uses_white_and_precomposed_opacity();
      result != 0) {
    return result;
  }
  if (const int result =
          test_authored_tint_multiplies_sample_and_opacity_once();
      result != 0) {
    return result;
  }
  if (const int result = test_vertex_and_shader_color_contract(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
