#include "vulkan_glyph_atlas_uploads_internal.hpp"

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

int test_dirty_region_copy_shape() {
  const cgpui::GlyphAtlasUploadRegion upload{
      .atlas_bounds =
          cgpui::Rect{
              .origin = cgpui::Point{.x = 12.0F, .y = 18.0F},
              .size = cgpui::Size{.width = 7.0F, .height = 9.0F},
          },
      .width = 7,
      .height = 9,
      .stride = 7,
      .byte_offset = 64,
      .byte_size = 63,
  };
  const VkBufferImageCopy copy =
      cgpui::vulkan_glyph_atlas_buffer_image_copy(upload, 32);
  return copy.bufferOffset == 32 && copy.bufferRowLength == 0 &&
                 copy.bufferImageHeight == 0 &&
                 copy.imageSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT &&
                 copy.imageSubresource.layerCount == 1 &&
                 copy.imageOffset.x == 12 && copy.imageOffset.y == 18 &&
                 copy.imageExtent.width == 7 && copy.imageExtent.height == 9 &&
                 copy.imageExtent.depth == 1
             ? 0
             : 10;
}

int test_layout_transition_shapes() {
  const VkImageMemoryBarrier first =
      cgpui::vulkan_glyph_atlas_image_barrier(
          VK_NULL_HANDLE,
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  if (first.sType != VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER ||
      first.srcAccessMask != 0 ||
      first.dstAccessMask != VK_ACCESS_TRANSFER_WRITE_BIT ||
      first.subresourceRange.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT ||
      cgpui::vulkan_glyph_atlas_source_stage(first.oldLayout) !=
          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) {
    return 11;
  }

  const VkImageMemoryBarrier repeated =
      cgpui::vulkan_glyph_atlas_image_barrier(
          VK_NULL_HANDLE,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  if (repeated.srcAccessMask != VK_ACCESS_SHADER_READ_BIT ||
      repeated.dstAccessMask != VK_ACCESS_TRANSFER_WRITE_BIT ||
      cgpui::vulkan_glyph_atlas_source_stage(repeated.oldLayout) !=
          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT) {
    return 12;
  }

  const VkImageMemoryBarrier readable =
      cgpui::vulkan_glyph_atlas_image_barrier(
          VK_NULL_HANDLE,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  return readable.srcAccessMask == VK_ACCESS_TRANSFER_WRITE_BIT &&
                 readable.dstAccessMask == VK_ACCESS_SHADER_READ_BIT
             ? 0
             : 13;
}

int test_upload_module_structure() {
  const std::string internal = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_uploads_internal.hpp");
  const std::string staging = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_staging.cpp");
  const std::string recording = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_upload_recording.cpp");
  const std::string memory =
      read_source("src/renderer/vulkan/vulkan_device_memory.cpp");
  const std::string command_header = read_source(
      "src/renderer/vulkan/vulkan_command_recording_internal.hpp");
  const std::string command_source =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_frame.cpp");
  const std::string state =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  if (internal.empty() || staging.empty() || recording.empty() ||
      memory.empty() || command_header.empty() || command_source.empty() ||
      frame.empty() || state.empty()) {
    return 20;
  }
  if (!contains(internal, "struct VulkanGlyphAtlasStagingUpload") ||
      !contains(internal, "struct VulkanGlyphAtlasUploadResources") ||
      !contains(internal, "VkBuffer buffer") ||
      !contains(internal, "VkDeviceMemory memory") ||
      !contains(staging, "VK_BUFFER_USAGE_TRANSFER_SRC_BIT") ||
      !contains(staging, "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT") ||
      !contains(staging, "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT") ||
      !contains(staging, "vkMapMemory") ||
      !contains(staging, "std::memcpy")) {
    return 21;
  }
  if (!contains(recording, "vkCmdPipelineBarrier") ||
      !contains(recording, "vkCmdCopyBufferToImage") ||
      !contains(recording, "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL") ||
      !contains(recording, "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL") ||
      !contains(memory, "vkGetPhysicalDeviceMemoryProperties")) {
    return 22;
  }
  if (!contains(command_header, "VulkanGlyphAtlasUploadResources") ||
      !contains(command_source, "vulkan_record_glyph_atlas_uploads(") ||
      !contains(frame, "vulkan_stage_glyph_atlas_uploads(") ||
      !contains(frame, "vulkan_commit_glyph_atlas_uploads(") ||
      !contains(state,
                "VulkanGlyphAtlasUploadResources glyph_atlas_uploads_")) {
    return 23;
  }
  return 0;
}

int test_step_461_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 461",
      "VulkanGlyphAtlasUploadResources",
      "vulkan_glyph_atlas_staging.cpp",
      "vkCmdCopyBufferToImage",
      "4-byte-aligned",
      "Step 462",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 30;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_dirty_region_copy_shape(); result != 0) {
    return result;
  }
  if (const int result = test_layout_transition_shapes(); result != 0) {
    return result;
  }
  if (const int result = test_upload_module_structure(); result != 0) {
    return result;
  }
  return test_step_461_documentation();
}
