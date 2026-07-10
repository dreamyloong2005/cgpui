#include "vulkan_image_texture_uploads_internal.hpp"

#include "cgpui/renderer/renderer_frame.hpp"

#include <array>
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

cgpui::ImageAsset image_asset(std::uint32_t stride = 8) {
  return cgpui::ImageAsset{
      .id = cgpui::ImageAssetId{17},
      .logical_size = {.width = 2.0F, .height = 2.0F},
      .bitmap =
          cgpui::DecodedImageBitmap{
              .width = 2,
              .height = 2,
              .stride = stride,
              .format = cgpui::ImageFormat::rgba8_unorm,
              .pixels = std::vector<std::uint8_t>(stride * 2, 0x7f),
          },
  };
}

class RecordingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  void upload_image(const cgpui::ImageAsset& image) override {
    uploaded = cgpui::describe_image_asset(image);
    upload_count += 1;
  }
  cgpui::Result<void> present() override { return {}; }

  std::size_t upload_count = 0;
  cgpui::ImageAssetDescriptor uploaded;
};

int test_explicit_frame_upload_transport() {
  RecordingFrame frame;
  const cgpui::ImageAsset image = image_asset();
  frame.upload_image(image);
  return frame.upload_count == 1 && frame.uploaded.id == image.id &&
                 frame.uploaded.byte_size == image.bitmap.pixels.size()
             ? 0
             : 10;
}

int test_rgba_upload_validation_and_copy() {
  const cgpui::ImageAsset image = image_asset();
  const std::array<cgpui::ImageAsset, 1> assets{image};
  const std::vector<cgpui::ImageUploadBatch> batches =
      cgpui::vulkan_plan_image_uploads(assets);
  if (batches.size() != 1 ||
      !cgpui::vulkan_image_texture_upload_batch_valid(batches.front())) {
    return 20;
  }
  const VkBufferImageCopy copy =
      cgpui::vulkan_image_texture_buffer_image_copy(
          batches.front().uploads.front(),
          0);
  if (copy.bufferOffset != 0 || copy.bufferRowLength != 0 ||
      copy.bufferImageHeight != 0 ||
      copy.imageSubresource.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT ||
      copy.imageExtent.width != 2 || copy.imageExtent.height != 2 ||
      copy.imageExtent.depth != 1) {
    return 21;
  }

  const cgpui::ImageAsset padded = image_asset(12);
  const std::array<cgpui::ImageAsset, 1> padded_assets{padded};
  const auto padded_batches = cgpui::vulkan_plan_image_uploads(padded_assets);
  const VkBufferImageCopy padded_copy =
      cgpui::vulkan_image_texture_buffer_image_copy(
          padded_batches.front().uploads.front(),
          4);
  if (padded_copy.bufferOffset != 4 || padded_copy.bufferRowLength != 3) {
    return 22;
  }

  cgpui::ImageUploadBatch invalid = batches.front();
  invalid.rgba.pop_back();
  if (cgpui::vulkan_image_texture_upload_batch_valid(invalid)) {
    return 23;
  }
  invalid = batches.front();
  invalid.uploads.front().byte_offset = 1;
  invalid.uploads.front().byte_size -= 1;
  return cgpui::vulkan_image_texture_upload_batch_valid(invalid) ? 24 : 0;
}

int test_upload_layout_barriers() {
  const VkImage image = fake_handle<VkImage>(31);
  const VkImageMemoryBarrier to_transfer =
      cgpui::vulkan_image_texture_upload_barrier(
          image,
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  const VkImageMemoryBarrier to_readable =
      cgpui::vulkan_image_texture_upload_barrier(
          image,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  if (cgpui::vulkan_image_texture_upload_source_stage(
          VK_IMAGE_LAYOUT_UNDEFINED) != VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ||
      to_transfer.image != image ||
      to_transfer.oldLayout != VK_IMAGE_LAYOUT_UNDEFINED ||
      to_transfer.newLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
      to_transfer.dstAccessMask != VK_ACCESS_TRANSFER_WRITE_BIT ||
      to_readable.srcAccessMask != VK_ACCESS_TRANSFER_WRITE_BIT ||
      to_readable.dstAccessMask != VK_ACCESS_SHADER_READ_BIT ||
      to_readable.newLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    return 30;
  }
  return 0;
}

int test_structure_and_documentation() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_image_texture_uploads_internal.hpp");
  const std::string staging =
      read_source("src/renderer/vulkan/vulkan_image_texture_staging.cpp");
  const std::string recording =
      read_source("src/renderer/vulkan/vulkan_image_texture_upload_recording.cpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_image_texture_frame.cpp");
  const std::string renderer_header =
      read_source("include/cgpui/renderer/renderer_frame.hpp");
  const std::string renderer =
      read_source("src/renderer/vulkan/vulkan_renderer.cpp");
  const std::string command_recording =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string state =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  const std::string lifetime =
      read_source("tests/renderer/vulkan_frame_lifetime_test.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (header.empty() || staging.empty() || recording.empty() || frame.empty() ||
      renderer_header.empty() || renderer.empty() || command_recording.empty() ||
      presentation.empty() || state.empty() || lifetime.empty() ||
      structure.empty()) {
    return 40;
  }
  if (!contains(header, "struct VulkanImageTextureStagingUpload") ||
      !contains(header, "struct VulkanImageTextureUploadResources") ||
      !contains(staging, "VK_BUFFER_USAGE_TRANSFER_SRC_BIT") ||
      !contains(staging, "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT") ||
      !contains(staging, "vkMapMemory") ||
      !contains(recording, "vkCmdCopyBufferToImage") ||
      !contains(recording, "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL") ||
      !contains(frame, "vulkan_stage_image_texture_uploads(") ||
      !contains(renderer_header, "upload_image(const ImageAsset& image)") ||
      !contains(renderer, "VulkanFrame::upload_image(") ||
      !contains(command_recording, "vulkan_record_image_texture_uploads(") ||
      !contains(presentation, "commit_image_texture_frame()") ||
      !contains(state, "VulkanImageTextureUploadResources image_texture_uploads_") ||
      !contains(lifetime, "test_image_texture_upload_frame(") ||
      !contains(structure, "vulkan_image_texture_staging.cpp")) {
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
      "Phase E Step 492",
      "RenderFrame::upload_image",
      "host-visible RGBA staging",
      "buffer-to-image copy",
      "Step 493",
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
  if (const int result = test_explicit_frame_upload_transport(); result != 0) {
    return result;
  }
  if (const int result = test_rgba_upload_validation_and_copy(); result != 0) {
    return result;
  }
  if (const int result = test_upload_layout_barriers(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
