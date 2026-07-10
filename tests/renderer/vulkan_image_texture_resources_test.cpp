#include "vulkan_image_texture_resources_internal.hpp"

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

cgpui::ImageAssetDescriptor descriptor(std::uint64_t id = 7) {
  return cgpui::ImageAssetDescriptor{
      .id = cgpui::ImageAssetId{id},
      .logical_size = {.width = 8.0F, .height = 6.0F},
      .pixel_width = 4,
      .pixel_height = 3,
      .stride = 16,
      .format = cgpui::ImageFormat::rgba8_unorm,
      .byte_size = 48,
  };
}

int test_rgba_texture_create_infos() {
  const cgpui::ImageAssetDescriptor image = descriptor();
  if (!cgpui::vulkan_image_texture_descriptor_valid(image) ||
      cgpui::vulkan_image_texture_format(image.format) !=
          VK_FORMAT_R8G8B8A8_UNORM) {
    return 10;
  }

  const VkImageCreateInfo create_info =
      cgpui::vulkan_image_texture_create_info(image);
  if (create_info.imageType != VK_IMAGE_TYPE_2D ||
      create_info.format != VK_FORMAT_R8G8B8A8_UNORM ||
      create_info.extent.width != 4 || create_info.extent.height != 3 ||
      create_info.extent.depth != 1 || create_info.mipLevels != 1 ||
      create_info.arrayLayers != 1 ||
      create_info.samples != VK_SAMPLE_COUNT_1_BIT ||
      create_info.tiling != VK_IMAGE_TILING_OPTIMAL ||
      (create_info.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0 ||
      (create_info.usage & VK_IMAGE_USAGE_SAMPLED_BIT) == 0 ||
      create_info.sharingMode != VK_SHARING_MODE_EXCLUSIVE ||
      create_info.initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
    return 11;
  }

  const VkImage image_handle = fake_handle<VkImage>(17);
  const VkImageViewCreateInfo view_info =
      cgpui::vulkan_image_texture_view_create_info(
          image_handle,
          VK_FORMAT_R8G8B8A8_UNORM);
  return view_info.image == image_handle &&
                 view_info.viewType == VK_IMAGE_VIEW_TYPE_2D &&
                 view_info.format == VK_FORMAT_R8G8B8A8_UNORM &&
                 view_info.subresourceRange.aspectMask ==
                     VK_IMAGE_ASPECT_COLOR_BIT &&
                 view_info.subresourceRange.levelCount == 1 &&
                 view_info.subresourceRange.layerCount == 1
             ? 0
             : 12;
}

int test_descriptor_identity_and_request_preflight() {
  const cgpui::ImageAssetDescriptor image = descriptor();
  cgpui::VulkanImageTextureResource resource{
      .descriptor = image,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .extent = {.width = 4, .height = 3},
  };
  cgpui::ImageAssetDescriptor upload_metadata_changed = image;
  upload_metadata_changed.stride = 32;
  upload_metadata_changed.byte_size = 96;
  if (!cgpui::vulkan_image_texture_resource_matches(
          resource,
          upload_metadata_changed)) {
    return 20;
  }
  cgpui::ImageAssetDescriptor resized = image;
  resized.pixel_width = 5;
  if (cgpui::vulkan_image_texture_resource_matches(resource, resized)) {
    return 21;
  }

  const std::array<cgpui::ImageDraw, 2> duplicates{
      cgpui::ImageDraw{.asset = image},
      cgpui::ImageDraw{.asset = image},
  };
  if (!cgpui::vulkan_validate_image_texture_resource_requests(duplicates)) {
    return 22;
  }
  const std::array<cgpui::ImageDraw, 2> conflicting{
      cgpui::ImageDraw{.asset = image},
      cgpui::ImageDraw{.asset = resized},
  };
  if (cgpui::vulkan_validate_image_texture_resource_requests(conflicting)) {
    return 23;
  }
  cgpui::ImageAssetDescriptor invalid = image;
  invalid.id = {};
  const std::array<cgpui::ImageDraw, 1> invalid_request{
      cgpui::ImageDraw{.asset = invalid},
  };
  return cgpui::vulkan_validate_image_texture_resource_requests(invalid_request)
             ? 24
             : 0;
}

int test_structure_and_documentation() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_image_texture_resources_internal.hpp");
  const std::string images =
      read_source("src/renderer/vulkan/vulkan_image_texture_images.cpp");
  const std::string resources =
      read_source("src/renderer/vulkan/vulkan_image_texture_resources.cpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_image_texture_frame.cpp");
  const std::string renderer =
      read_source("src/renderer/vulkan/vulkan_renderer.cpp");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string state =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  const std::string state_source =
      read_source("src/renderer/vulkan/vulkan_state.cpp");
  const std::string lifetime =
      read_source("tests/renderer/vulkan_frame_lifetime_test.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (header.empty() || images.empty() || resources.empty() || frame.empty() ||
      renderer.empty() || presentation.empty() || state.empty() ||
      state_source.empty() || lifetime.empty() || structure.empty()) {
    return 30;
  }
  if (!contains(header, "struct VulkanImageTextureResource") ||
      !contains(header, "struct VulkanImageTextureResources") ||
      !contains(images, "vkCreateImage") ||
      !contains(images, "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT") ||
      !contains(images, "vkCreateImageView") ||
      !contains(images, "vulkan_destroy_image_texture_resource(") ||
      !contains(resources, "vulkan_update_image_texture_resources(") ||
      !contains(resources, "vulkan_find_image_texture_resource(") ||
      !contains(frame, "prepare_image_texture_frame(") ||
      !contains(renderer, "image_draws_") ||
      !contains(renderer, "image_uploads_)") ||
      !contains(
          presentation,
          "prepare_image_texture_frame(image_draws, image_uploads)") ||
      !contains(state, "VulkanImageTextureResources image_texture_resources_") ||
      !contains(state_source, "vulkan_destroy_image_texture_resources(") ||
      !contains(lifetime, "test_image_texture_resource_frame(") ||
      !contains(structure, "vulkan_image_texture_images.cpp") ||
      !contains(structure, "vulkan_image_texture_resources.cpp") ||
      !contains(structure, "vulkan_image_texture_frame.cpp")) {
    return 31;
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
      "Phase E Step 491",
      "VulkanImageTextureResources",
      "device-local RGBA image/view ownership",
      "Step 492",
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
  if (const int result = test_rgba_texture_create_infos(); result != 0) {
    return result;
  }
  if (const int result = test_descriptor_identity_and_request_preflight();
      result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
