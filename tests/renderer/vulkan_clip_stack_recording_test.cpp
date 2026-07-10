#include "vulkan_clip_scissor_internal.hpp"
#include "vulkan_rounded_rect_geometry_internal.hpp"
#include "vulkan_text_draw_recording_internal.hpp"

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

cgpui::Rect rect(float x, float y, float width, float height) {
  return cgpui::Rect{
      .origin = {.x = x, .y = y},
      .size = {.width = width, .height = height},
  };
}

int test_full_frame_and_nested_clip_scissors() {
  const VkExtent2D extent{.width = 100, .height = 80};
  const auto full = cgpui::vulkan_resolve_clip_stack_scissor(
      extent, std::nullopt, cgpui::RendererClipStackRecord{});
  if (!full.visible || full.clipped || full.scissor.offset.x != 0 ||
      full.scissor.offset.y != 0 || full.scissor.extent.width != 100 ||
      full.scissor.extent.height != 80) {
    return 10;
  }

  cgpui::RendererClipStackRecord stack{
      .clips = {rect(5.0F, 4.0F, 90.0F, 70.0F),
                rect(20.25F, 14.75F, 60.5F, 40.25F)},
      .full_depth = 2,
      .current_clip_rect = rect(20.25F, 14.75F, 60.5F, 40.25F),
  };
  const auto nested = cgpui::vulkan_resolve_clip_stack_scissor(
      extent, rect(0.0F, 0.0F, 100.0F, 80.0F), stack);
  if (!nested.visible || !nested.clipped || nested.scissor.offset.x != 20 ||
      nested.scissor.offset.y != 14 || nested.scissor.extent.width != 61 ||
      nested.scissor.extent.height != 41) {
    return 11;
  }
  return 0;
}

int test_empty_clip_is_not_visible() {
  cgpui::RendererClipStackRecord stack{
      .clips = {rect(120.0F, 90.0F, 20.0F, 10.0F)},
      .full_depth = 1,
      .current_clip_rect = rect(120.0F, 90.0F, 20.0F, 10.0F),
  };
  const auto resolved = cgpui::vulkan_resolve_clip_stack_scissor(
      VkExtent2D{.width = 100, .height = 80}, std::nullopt, stack);
  return !resolved.visible && resolved.clipped ? 0 : 20;
}

int test_rounded_geometry_preserves_effective_clip() {
  cgpui::RendererClipStackRecord stack{
      .clips = {rect(4.0F, 5.0F, 70.0F, 60.0F),
                rect(10.0F, 12.0F, 30.0F, 25.0F)},
      .full_depth = 2,
      .current_clip_rect = rect(10.0F, 12.0F, 30.0F, 25.0F),
  };
  const std::array<cgpui::RoundedRectDraw, 1> draws{
      cgpui::RoundedRectDraw{
          .rect = rect(0.0F, 0.0F, 80.0F, 70.0F),
          .color = {.r = 0.1F, .g = 0.2F, .b = 0.3F, .a = 1.0F},
          .radius = cgpui::BorderRadii::all(4.0F),
          .clip_rect = rect(8.0F, 8.0F, 50.0F, 50.0F),
          .clip_stack = stack,
      },
  };
  const cgpui::VulkanRoundedRectGeometry geometry =
      cgpui::vulkan_build_rounded_rect_geometry(draws, 2);
  if (geometry.draws.size() != 1 ||
      !geometry.draws[0].clip_rect.has_value()) {
    return 30;
  }
  const cgpui::Rect clip = *geometry.draws[0].clip_rect;
  return clip.origin.x == 10.0F && clip.origin.y == 12.0F &&
                 clip.size.width == 30.0F && clip.size.height == 25.0F
             ? 0
             : 31;
}

int test_text_draw_commands_preserve_effective_clip() {
  const std::array<cgpui::VulkanGlyphAtlasDrawBinding, 1> bindings{
      cgpui::VulkanGlyphAtlasDrawBinding{
          .page_index = 0,
          .first_quad_index = 0,
          .glyph_quad_count = 1,
          .descriptor_set = fake_handle<VkDescriptorSet>(1),
          .clip_rect = rect(9.0F, 11.0F, 40.0F, 22.0F),
      },
  };
  const cgpui::VulkanTextPipelineResources pipeline{
      .layout = fake_handle<VkPipelineLayout>(2),
      .pipeline = fake_handle<VkPipeline>(3),
  };
  const cgpui::VulkanTextVertexBufferResources vertices{
      .vertices =
          cgpui::VulkanFrameGeometryBufferResources{
              .buffer = fake_handle<VkBuffer>(4),
              .memory = fake_handle<VkDeviceMemory>(5),
              .byte_capacity = 6 * sizeof(cgpui::VulkanTextVertex),
              .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
          },
      .vertex_count = 6,
      .byte_size = 6 * sizeof(cgpui::VulkanTextVertex),
  };
  const auto commands =
      cgpui::vulkan_plan_text_draw_commands(bindings, pipeline, vertices);
  if (!commands || commands->size() != 1 ||
      !commands->front().clip_rect.has_value()) {
    return 40;
  }
  const cgpui::Rect clip = *commands->front().clip_rect;
  return clip.origin.x == 9.0F && clip.origin.y == 11.0F &&
                 clip.size.width == 40.0F && clip.size.height == 22.0F
             ? 0
             : 41;
}

int test_recording_structure_and_documentation() {
  const std::string clip_header = read_source(
      "src/renderer/vulkan/vulkan_clip_scissor_internal.hpp");
  const std::string clip_source =
      read_source("src/renderer/vulkan/vulkan_clip_scissor.cpp");
  const std::string solid_source =
      read_source("src/renderer/vulkan/vulkan_solid_rect_geometry.cpp");
  const std::string rounded_source = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_draw_recording.cpp");
  const std::string text_source =
      read_source("src/renderer/vulkan/vulkan_text_draw_recording.cpp");
  if (clip_header.empty() || clip_source.empty() || solid_source.empty() ||
      rounded_source.empty() || text_source.empty()) {
    return 50;
  }
  const char* clip_required[] = {
      "VulkanClipScissorResolution",
      "vulkan_resolve_effective_clip_rect(",
      "vulkan_resolve_clip_stack_scissor(",
  };
  for (const char* value : clip_required) {
    if (!contains(clip_header, value) && !contains(clip_source, value)) {
      return 51;
    }
  }
  if (!contains(solid_source, "vulkan_resolve_effective_clip_rect(") ||
      !contains(rounded_source, "vulkan_resolve_clip_stack_scissor(") ||
      !contains(text_source, "vulkan_resolve_clip_stack_scissor(") ||
      !contains(rounded_source, "vkCmdSetScissor") ||
      !contains(text_source, "vkCmdSetScissor")) {
    return 52;
  }

  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* docs_required[] = {
      "Phase E Step 483",
      "vulkan_resolve_clip_stack_scissor",
      "per-draw dynamic scissor",
      "Step 484",
  };
  for (const char* value : docs_required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 60;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_full_frame_and_nested_clip_scissors();
      result != 0) {
    return result;
  }
  if (const int result = test_empty_clip_is_not_visible(); result != 0) {
    return result;
  }
  if (const int result = test_rounded_geometry_preserves_effective_clip();
      result != 0) {
    return result;
  }
  if (const int result = test_text_draw_commands_preserve_effective_clip();
      result != 0) {
    return result;
  }
  return test_recording_structure_and_documentation();
}
