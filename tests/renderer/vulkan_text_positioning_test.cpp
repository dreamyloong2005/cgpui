#include "vulkan_text_positioning_internal.hpp"
#include "vulkan_text_vertex_buffer_internal.hpp"

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

cgpui::TexturedGlyphQuad fractional_quad() {
  return cgpui::TexturedGlyphQuad{
      .device_bounds =
          cgpui::Rect{
              .origin = cgpui::Point{.x = 1.25F, .y = 2.5F},
              .size = cgpui::Size{.width = 3.75F, .height = 4.5F},
          },
      .atlas_uv_bounds =
          cgpui::Rect{
              .origin = cgpui::Point{.x = 0.1F, .y = 0.2F},
              .size = cgpui::Size{.width = 0.3F, .height = 0.4F},
          },
      .color = cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
  };
}

int test_positioning_policy_contract() {
  using enum cgpui::VulkanTextPositioningPolicy;
  const cgpui::VulkanTextVertexBufferResources default_resources;
  if (cgpui::vulkan_default_text_positioning_policy() != preserve_subpixel ||
      default_resources.positioning_policy != preserve_subpixel) {
    return 10;
  }
  const cgpui::Rect bounds = fractional_quad().device_bounds;
  const cgpui::Rect preserved =
      cgpui::vulkan_position_text_bounds(bounds, preserve_subpixel);
  const cgpui::Rect snapped =
      cgpui::vulkan_position_text_bounds(bounds, snap_to_device_pixels);
  if (preserved.origin.x != bounds.origin.x ||
      preserved.origin.y != bounds.origin.y ||
      preserved.size.width != bounds.size.width ||
      preserved.size.height != bounds.size.height ||
      snapped.origin.x != 1.0F || snapped.origin.y != 3.0F ||
      snapped.size.width != 4.0F || snapped.size.height != 4.0F) {
    return 11;
  }
  return 0;
}

int test_vertex_expansion_consumes_policy() {
  using enum cgpui::VulkanTextPositioningPolicy;
  const std::array<cgpui::TexturedGlyphQuad, 1> quads{fractional_quad()};
  const auto preserved =
      cgpui::vulkan_build_text_vertices(quads, preserve_subpixel);
  const auto snapped =
      cgpui::vulkan_build_text_vertices(quads, snap_to_device_pixels);
  if (preserved.size() != 6 || snapped.size() != 6 ||
      preserved.front().position != std::array<float, 2>{1.25F, 2.5F} ||
      snapped.front().position != std::array<float, 2>{1.0F, 3.0F} ||
      snapped[2].position != std::array<float, 2>{5.0F, 7.0F} ||
      snapped.front().atlas_uv != preserved.front().atlas_uv ||
      snapped[2].atlas_uv != preserved[2].atlas_uv) {
    return 20;
  }
  return 0;
}

int test_positioning_module_structure() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_text_positioning_internal.hpp");
  const std::string source =
      read_source("src/renderer/vulkan/vulkan_text_positioning.cpp");
  const std::string vertices =
      read_source("src/renderer/vulkan/vulkan_text_vertex_buffer.cpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_frame.cpp");
  if (header.empty() || source.empty() || vertices.empty() || frame.empty()) {
    return 30;
  }
  if (!contains(header, "enum class VulkanTextPositioningPolicy") ||
      !contains(source, "vulkan_position_text_bounds(") ||
      !contains(source, "std::round") ||
      !contains(vertices, "vulkan_position_text_bounds(") ||
      !contains(frame, "vulkan_default_text_positioning_policy(")) {
    return 31;
  }
  return 0;
}

int test_step_472_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 472",
      "VulkanTextPositioningPolicy",
      "preserve_subpixel",
      "Step 473",
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
  if (const int result = test_positioning_policy_contract(); result != 0) {
    return result;
  }
  if (const int result = test_vertex_expansion_consumes_policy(); result != 0) {
    return result;
  }
  if (const int result = test_positioning_module_structure(); result != 0) {
    return result;
  }
  return test_step_472_documentation();
}
