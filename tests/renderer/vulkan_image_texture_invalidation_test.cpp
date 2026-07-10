#include "vulkan_image_texture_invalidation_internal.hpp"

#include "cgpui/renderer/renderer_frame.hpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

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

cgpui::VulkanImageTextureResource resource(std::uint64_t id) {
  return cgpui::VulkanImageTextureResource{
      .descriptor = cgpui::ImageAssetDescriptor{
          .id = cgpui::ImageAssetId{id},
      },
  };
}

class RecordingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  void invalidate_image(cgpui::ImageAssetId asset_id) override {
    invalidations.push_back(asset_id);
  }
  cgpui::Result<void> present() override { return {}; }

  std::vector<cgpui::ImageAssetId> invalidations;
};

int test_public_frame_invalidation_command() {
  RecordingFrame frame;
  frame.invalidate_image(cgpui::ImageAssetId{7});
  return frame.invalidations.size() == 1 &&
                 frame.invalidations[0].value == 7
             ? 0
             : 10;
}

int test_resource_invalidation_is_idempotent() {
  cgpui::VulkanImageTextureResources resources;
  resources.textures = {resource(1), resource(2), resource(3)};
  const std::array<cgpui::ImageAssetId, 4> invalidations{
      cgpui::ImageAssetId{2},
      cgpui::ImageAssetId{2},
      cgpui::ImageAssetId{99},
      cgpui::ImageAssetId{},
  };
  const std::size_t invalidated =
      cgpui::vulkan_invalidate_image_texture_resources(
          VK_NULL_HANDLE, invalidations, resources);
  return invalidated == 1 && resources.textures.size() == 2 &&
                 resources.textures[0].descriptor.id.value == 1 &&
                 resources.textures[1].descriptor.id.value == 3
             ? 0
             : 20;
}

int test_structure_and_documentation() {
  const std::string renderer_frame =
      read_source("include/cgpui/renderer/renderer_frame.hpp");
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_image_texture_invalidation_internal.hpp");
  const std::string invalidation = read_source(
      "src/renderer/vulkan/vulkan_image_texture_invalidation.cpp");
  const std::string renderer =
      read_source("src/renderer/vulkan/vulkan_renderer.cpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_image_texture_frame.cpp");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string lifetime =
      read_source("tests/renderer/vulkan_frame_lifetime_test.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (renderer_frame.empty() || header.empty() || invalidation.empty() ||
      renderer.empty() || frame.empty() || presentation.empty() ||
      lifetime.empty() || structure.empty()) {
    return 30;
  }
  const std::size_t invalidate =
      frame.find("vulkan_invalidate_image_texture_resources(");
  const std::size_t prepare =
      frame.find("vulkan_prepare_image_texture_cache_frame(");
  const std::size_t wait = presentation.find("vkWaitForFences");
  const std::size_t prepare_frame =
      presentation.find("prepare_image_texture_frame(");
  if (!contains(renderer_frame, "invalidate_image(ImageAssetId asset_id)") ||
      !contains(header, "vulkan_invalidate_image_texture_resources(") ||
      !contains(invalidation, "vulkan_destroy_image_texture_resource(") ||
      !contains(renderer, "VulkanFrame::invalidate_image(") ||
      !contains(renderer, "image_invalidations_") ||
      invalidate == std::string::npos || prepare == std::string::npos ||
      invalidate >= prepare || wait == std::string::npos ||
      prepare_frame == std::string::npos || wait >= prepare_frame ||
      !contains(lifetime, "test_image_texture_invalidation_frame(") ||
      !contains(structure, "vulkan_image_texture_invalidation.cpp")) {
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
      "Phase E Step 497",
      "RenderFrame::invalidate_image",
      "deduplicated image invalidations",
      "fence-safe resource destruction",
      "Step 498 image integration closeout",
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
  if (const int result = test_public_frame_invalidation_command(); result != 0) {
    return result;
  }
  if (const int result = test_resource_invalidation_is_idempotent();
      result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
