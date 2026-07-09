#include "vulkan_text_vertex_buffer_internal.hpp"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <type_traits>

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

template <typename Handle>
Handle fake_handle(std::uintptr_t value) {
  if constexpr (std::is_pointer_v<Handle>) {
    return reinterpret_cast<Handle>(value);
  } else {
    return static_cast<Handle>(value);
  }
}

bool vertex_equals(
    const cgpui::VulkanTextVertex& vertex,
    std::array<float, 2> position,
    std::array<float, 2> atlas_uv,
    std::array<float, 4> color) {
  return vertex.position == position && vertex.atlas_uv == atlas_uv &&
         vertex.color == color;
}

int test_quad_expands_to_triangle_vertices() {
  const cgpui::TexturedGlyphQuad quad{
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
      .color = cgpui::Color{.r = 0.5F, .g = 0.6F, .b = 0.7F, .a = 0.8F},
  };
  const std::array<cgpui::TexturedGlyphQuad, 1> quads{quad};
  const std::vector<cgpui::VulkanTextVertex> vertices =
      cgpui::vulkan_build_text_vertices(quads);
  const std::array<float, 4> color{0.5F, 0.6F, 0.7F, 0.8F};
  if (vertices.size() != cgpui::vulkan_text_vertices_per_quad ||
      !vertex_equals(vertices[0], {1.25F, 2.5F}, {0.1F, 0.2F}, color) ||
      !vertex_equals(vertices[1], {5.0F, 2.5F}, {0.4F, 0.2F}, color) ||
      !vertex_equals(vertices[2], {5.0F, 7.0F}, {0.4F, 0.6F}, color) ||
      !vertex_equals(vertices[3], {1.25F, 2.5F}, {0.1F, 0.2F}, color) ||
      !vertex_equals(vertices[4], {5.0F, 7.0F}, {0.4F, 0.6F}, color) ||
      !vertex_equals(vertices[5], {1.25F, 7.0F}, {0.1F, 0.6F}, color)) {
    return 10;
  }
  return 0;
}

int test_vertex_buffer_contract() {
  cgpui::VulkanTextVertexBufferResources resources;
  if (resources.ready()) {
    return 20;
  }
  resources.buffer = fake_handle<VkBuffer>(1);
  resources.memory = fake_handle<VkDeviceMemory>(2);
  resources.vertex_count = 6;
  resources.byte_size = 6 * sizeof(cgpui::VulkanTextVertex);
  if (!resources.ready()) {
    return 21;
  }

  const VkBufferCreateInfo create_info =
      cgpui::vulkan_text_vertex_buffer_create_info(resources.byte_size);
  if (create_info.sType != VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO ||
      create_info.size != resources.byte_size ||
      create_info.usage != VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ||
      create_info.sharingMode != VK_SHARING_MODE_EXCLUSIVE) {
    return 22;
  }
  return 0;
}

int test_vertex_buffer_structure() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_text_vertex_buffer_internal.hpp");
  const std::string source =
      read_source("src/renderer/vulkan/vulkan_text_vertex_buffer.cpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_frame.cpp");
  const std::string state =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  if (header.empty() || source.empty() || frame.empty() || state.empty()) {
    return 30;
  }
  if (!contains(header, "struct VulkanTextVertexBufferResources") ||
      !contains(source, "vulkan_build_text_vertices(") ||
      !contains(source, "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT") ||
      !contains(source, "vkMapMemory") ||
      !contains(frame, "vulkan_upload_text_vertex_buffer(") ||
      !contains(state, "VulkanTextVertexBufferResources text_vertex_buffer_")) {
    return 31;
  }
  return 0;
}

int test_step_470_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 470",
      "VulkanTextVertexBufferResources",
      "vulkan_upload_text_vertex_buffer",
      "Step 471",
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
  if (const int result = test_quad_expands_to_triangle_vertices(); result != 0) {
    return result;
  }
  if (const int result = test_vertex_buffer_contract(); result != 0) {
    return result;
  }
  if (const int result = test_vertex_buffer_structure(); result != 0) {
    return result;
  }
  return test_step_470_documentation();
}
