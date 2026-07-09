#include "vulkan_rounded_rect_buffers_internal.hpp"

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

int test_rounded_rect_buffer_resource_contract() {
  cgpui::VulkanRoundedRectBufferResources resources;
  if (resources.ready()) {
    return 10;
  }
  resources.vertex_buffer = fake_handle<VkBuffer>(1);
  resources.vertex_memory = fake_handle<VkDeviceMemory>(2);
  resources.index_buffer = fake_handle<VkBuffer>(3);
  resources.index_memory = fake_handle<VkDeviceMemory>(4);
  resources.vertex_count = 5;
  resources.index_count = 12;
  resources.vertex_byte_size =
      resources.vertex_count * sizeof(cgpui::VulkanRoundedRectVertex);
  resources.index_byte_size = resources.index_count * sizeof(std::uint32_t);
  resources.draws.push_back(cgpui::VulkanRoundedRectDrawRange{
      .vertex_count = 5,
      .index_count = 12,
  });
  return resources.ready() ? 0 : 11;
}

int test_rounded_rect_buffer_create_infos() {
  const VkBufferCreateInfo vertex = cgpui::vulkan_rounded_rect_buffer_create_info(
      256, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  const VkBufferCreateInfo index = cgpui::vulkan_rounded_rect_buffer_create_info(
      128, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  if (vertex.sType != VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO ||
      vertex.size != 256 ||
      vertex.usage != VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ||
      vertex.sharingMode != VK_SHARING_MODE_EXCLUSIVE ||
      index.size != 128 || index.usage != VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
    return 20;
  }
  return 0;
}

int test_rounded_rect_buffer_structure() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_buffers_internal.hpp");
  const std::string source =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_buffers.cpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_frame.cpp");
  const std::string state =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  const std::string destructor =
      read_source("src/renderer/vulkan/vulkan_state.cpp");
  if (header.empty() || source.empty() || frame.empty() || state.empty() ||
      destructor.empty()) {
    return 30;
  }
  if (!contains(header, "struct VulkanRoundedRectBufferResources") ||
      !contains(source, "vulkan_upload_rounded_rect_buffers(") ||
      !contains(source, "VK_BUFFER_USAGE_INDEX_BUFFER_BIT") ||
      !contains(source, "vkMapMemory") ||
      !contains(frame, "prepare_rounded_rect_frame(") ||
      !contains(state, "VulkanRoundedRectBufferResources rounded_rect_buffers_") ||
      !contains(destructor, "vulkan_destroy_rounded_rect_buffers(")) {
    return 31;
  }
  return 0;
}

int test_step_476_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 476",
      "VulkanRoundedRectBufferResources",
      "vulkan_upload_rounded_rect_buffers",
      "Step 477",
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
  if (const int result = test_rounded_rect_buffer_resource_contract();
      result != 0) {
    return result;
  }
  if (const int result = test_rounded_rect_buffer_create_infos(); result != 0) {
    return result;
  }
  if (const int result = test_rounded_rect_buffer_structure(); result != 0) {
    return result;
  }
  return test_step_476_documentation();
}
