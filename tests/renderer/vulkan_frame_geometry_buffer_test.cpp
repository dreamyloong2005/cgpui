#include "vulkan_frame_geometry_buffer_internal.hpp"

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

cgpui::VulkanFrameGeometryBufferResources allocated_vertex_buffer(
    std::size_t byte_capacity) {
  return cgpui::VulkanFrameGeometryBufferResources{
      .buffer = fake_handle<VkBuffer>(1),
      .memory = fake_handle<VkDeviceMemory>(2),
      .byte_capacity = byte_capacity,
      .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
  };
}

int test_empty_plan_retains_allocation() {
  const cgpui::VulkanFrameGeometryBufferResources resources =
      allocated_vertex_buffer(256);
  const cgpui::VulkanFrameGeometryBufferPlan plan =
      cgpui::vulkan_plan_frame_geometry_buffer(
          resources, 0, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  return plan.action ==
                     cgpui::VulkanFrameGeometryBufferAction::retain_empty &&
                 plan.required_byte_size == 0 &&
                 plan.allocation_byte_size == 256 && plan.valid()
             ? 0
             : 10;
}

int test_matching_capacity_is_reused() {
  const cgpui::VulkanFrameGeometryBufferPlan plan =
      cgpui::vulkan_plan_frame_geometry_buffer(
          allocated_vertex_buffer(256),
          128,
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  return plan.action == cgpui::VulkanFrameGeometryBufferAction::reuse &&
                 plan.required_byte_size == 128 &&
                 plan.allocation_byte_size == 256 && plan.valid()
             ? 0
             : 20;
}

int test_growth_replaces_with_geometric_capacity() {
  const cgpui::VulkanFrameGeometryBufferPlan plan =
      cgpui::vulkan_plan_frame_geometry_buffer(
          allocated_vertex_buffer(128),
          200,
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  return plan.action == cgpui::VulkanFrameGeometryBufferAction::replace &&
                 plan.required_byte_size == 200 &&
                 plan.allocation_byte_size == 256 && plan.valid()
             ? 0
             : 30;
}

int test_usage_change_replaces_allocation() {
  const cgpui::VulkanFrameGeometryBufferPlan plan =
      cgpui::vulkan_plan_frame_geometry_buffer(
          allocated_vertex_buffer(256),
          64,
          VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  if (plan.action != cgpui::VulkanFrameGeometryBufferAction::replace ||
      plan.required_byte_size != 64 || plan.allocation_byte_size != 64 ||
      plan.usage != VK_BUFFER_USAGE_INDEX_BUFFER_BIT || !plan.valid()) {
    return 40;
  }
  const cgpui::VulkanFrameGeometryBufferPlan invalid =
      cgpui::vulkan_plan_frame_geometry_buffer(
          {}, 64, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
  return !invalid.valid() ? 0 : 41;
}

int test_resource_contract() {
  cgpui::VulkanFrameGeometryBufferResources resources;
  if (resources.allocated()) {
    return 50;
  }
  resources = allocated_vertex_buffer(64);
  if (!resources.allocated()) {
    return 51;
  }
  const VkBufferCreateInfo create_info =
      cgpui::vulkan_frame_geometry_buffer_create_info(
          64, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  return create_info.sType == VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO &&
                 create_info.size == 64 &&
                 create_info.usage == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT &&
                 create_info.sharingMode == VK_SHARING_MODE_EXCLUSIVE
             ? 0
             : 52;
}

int test_primitive_owners_use_shared_leaf() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_frame_geometry_buffer_internal.hpp");
  const std::string source = read_source(
      "src/renderer/vulkan/vulkan_frame_geometry_buffer.cpp");
  const std::string text_header = read_source(
      "src/renderer/vulkan/vulkan_text_vertex_buffer_internal.hpp");
  const std::string text_source = read_source(
      "src/renderer/vulkan/vulkan_text_vertex_buffer.cpp");
  const std::string image_header = read_source(
      "src/renderer/vulkan/vulkan_image_vertex_buffer_internal.hpp");
  const std::string image_source = read_source(
      "src/renderer/vulkan/vulkan_image_vertex_buffer.cpp");
  const std::string rounded_header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_buffers_internal.hpp");
  const std::string rounded_source = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_buffers.cpp");
  const std::string structure = read_source(
      "tests/architecture/renderer_source_structure_test.cpp");
  if (header.empty() || source.empty() || text_header.empty() ||
      text_source.empty() || image_header.empty() || image_source.empty() ||
      rounded_header.empty() || rounded_source.empty() || structure.empty()) {
    return 60;
  }
  if (!contains(header, "struct VulkanFrameGeometryBufferResources") ||
      !contains(source, "vulkan_upload_frame_geometry_buffer(") ||
      !contains(text_header, "VulkanFrameGeometryBufferResources vertices") ||
      !contains(image_header, "VulkanFrameGeometryBufferResources vertices") ||
      !contains(rounded_header, "VulkanFrameGeometryBufferResources vertices") ||
      !contains(rounded_header, "VulkanFrameGeometryBufferResources indices") ||
      !contains(text_source, "vulkan_upload_frame_geometry_buffer(") ||
      !contains(image_source, "vulkan_upload_frame_geometry_buffer(") ||
      !contains(rounded_source, "vulkan_upload_frame_geometry_buffer(") ||
      !contains(structure, "vulkan_frame_geometry_buffer.cpp")) {
    return 61;
  }
  return 0;
}

int test_step_507_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 507",
      "VulkanFrameGeometryBufferResources",
      "reusable host-visible vertex/index buffers",
      "retained capacity",
      "Step 508 command reuse",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 70;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_empty_plan_retains_allocation(); result != 0) {
    return result;
  }
  if (const int result = test_matching_capacity_is_reused(); result != 0) {
    return result;
  }
  if (const int result = test_growth_replaces_with_geometric_capacity();
      result != 0) {
    return result;
  }
  if (const int result = test_usage_change_replaces_allocation(); result != 0) {
    return result;
  }
  if (const int result = test_resource_contract(); result != 0) {
    return result;
  }
  if (const int result = test_primitive_owners_use_shared_leaf(); result != 0) {
    return result;
  }
  return test_step_507_documentation();
}
