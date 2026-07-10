#include "vulkan_image_texture_cache_internal.hpp"

#include <cstdint>
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

cgpui::VulkanImageTextureResource resource(
    std::uint64_t id,
    std::uint64_t last_used_frame) {
  return cgpui::VulkanImageTextureResource{
      .descriptor = cgpui::ImageAssetDescriptor{
          .id = cgpui::ImageAssetId{id},
      },
      .last_used_frame = last_used_frame,
  };
}

int test_generation_and_touch() {
  cgpui::VulkanImageTextureResources resources;
  resources.textures.push_back(resource(1, 0));
  cgpui::vulkan_begin_image_texture_cache_frame(resources);
  if (resources.frame_index != 1) {
    return 10;
  }
  cgpui::vulkan_touch_image_texture_resource(
      resources, cgpui::ImageAssetId{1});
  if (resources.textures[0].last_used_frame != 1) {
    return 11;
  }
  cgpui::vulkan_begin_image_texture_cache_frame(resources);
  return resources.frame_index == 2 &&
                 !cgpui::vulkan_image_texture_resource_idle(
                     resources.textures[0],
                     resources.frame_index,
                     cgpui::vulkan_image_texture_cache_max_idle_frames)
             ? 0
             : 12;
}

int test_idle_boundary_and_eviction() {
  cgpui::VulkanImageTextureResources resources;
  resources.frame_index = 122;
  resources.textures = {
      resource(1, 1),
      resource(2, 2),
      resource(3, 122),
  };
  if (!cgpui::vulkan_image_texture_resource_idle(
          resources.textures[0],
          resources.frame_index,
          cgpui::vulkan_image_texture_cache_max_idle_frames) ||
      cgpui::vulkan_image_texture_resource_idle(
          resources.textures[1],
          resources.frame_index,
          cgpui::vulkan_image_texture_cache_max_idle_frames)) {
    return 20;
  }
  const std::size_t evicted =
      cgpui::vulkan_evict_idle_image_texture_resources(
          VK_NULL_HANDLE, resources);
  return evicted == 1 && resources.textures.size() == 2 &&
                 resources.textures[0].descriptor.id.value == 2 &&
                 resources.textures[1].descriptor.id.value == 3
             ? 0
             : 21;
}

int test_generation_wrap_rebases_live_resources() {
  cgpui::VulkanImageTextureResources resources;
  resources.frame_index = std::numeric_limits<std::uint64_t>::max();
  resources.textures.push_back(resource(4, resources.frame_index - 1));
  cgpui::vulkan_begin_image_texture_cache_frame(resources);
  return resources.frame_index == 1 &&
                 resources.textures[0].last_used_frame == 1
             ? 0
             : 30;
}

int test_structure_and_documentation() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_image_texture_cache_internal.hpp");
  const std::string cache =
      read_source("src/renderer/vulkan/vulkan_image_texture_cache.cpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_image_texture_frame.cpp");
  const std::string binding = read_source(
      "src/renderer/vulkan/vulkan_image_texture_resource_binding.cpp");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string lifetime =
      read_source("tests/renderer/vulkan_frame_lifetime_test.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (header.empty() || cache.empty() || frame.empty() || binding.empty() ||
      presentation.empty() || lifetime.empty() || structure.empty()) {
    return 40;
  }
  const std::size_t wait = presentation.find("vkWaitForFences");
  const std::size_t prepare =
      presentation.find("prepare_image_texture_frame(");
  if (!contains(header, "vulkan_image_texture_cache_max_idle_frames = 120") ||
      !contains(cache, "vulkan_begin_image_texture_cache_frame(") ||
      !contains(cache, "vulkan_touch_image_texture_resource(") ||
      !contains(cache, "vulkan_evict_idle_image_texture_resources(") ||
      !contains(frame, "vulkan_prepare_image_texture_cache_frame(") ||
      !contains(binding, "last_used_frame = resources.frame_index") ||
      wait == std::string::npos || prepare == std::string::npos ||
      wait >= prepare ||
      !contains(lifetime, "test_image_texture_cache_idle_frame(") ||
      !contains(structure, "vulkan_image_texture_cache.cpp")) {
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
      "Phase E Step 496",
      "frame-generation image cache",
      "120 idle frames",
      "fence-safe eviction",
      "Step 497 image invalidation",
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
  if (const int result = test_generation_and_touch(); result != 0) {
    return result;
  }
  if (const int result = test_idle_boundary_and_eviction(); result != 0) {
    return result;
  }
  if (const int result = test_generation_wrap_rebases_live_resources();
      result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
