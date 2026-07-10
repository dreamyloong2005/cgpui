#include "vulkan_frame_draw_order_internal.hpp"
#include "vulkan_text_draw_recording_internal.hpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <type_traits>
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

template <typename Handle>
Handle fake_handle(std::uintptr_t value) {
  if constexpr (std::is_pointer_v<Handle>) {
    return reinterpret_cast<Handle>(value);
  } else {
    return static_cast<Handle>(value);
  }
}

int test_compact_cursor_preserves_authored_interleaving() {
  static_assert(
      sizeof(cgpui::VulkanFrameDrawOrderEntry) <= 2 * sizeof(std::size_t));
  const std::array<cgpui::VulkanFrameDrawOrderEntry, 8> order{
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::rounded_rect,
          .command_index = 0,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
          .command_index = 0,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::text,
          .command_index = 0,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
          .command_index = 1,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::text,
          .command_index = 1,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::rounded_rect,
          .command_index = 1,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
          .command_index = 2,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::text_selection,
          .command_index = 0,
      },
  };
  const std::array<cgpui::VulkanRoundedRectDrawRange, 2> solid_draws{
      cgpui::VulkanRoundedRectDrawRange{.source_index = 0},
      cgpui::VulkanRoundedRectDrawRange{.source_index = 2},
  };
  const std::array<cgpui::VulkanRoundedRectDrawRange, 2> rounded_draws{
      cgpui::VulkanRoundedRectDrawRange{.source_index = 0},
      cgpui::VulkanRoundedRectDrawRange{.source_index = 1},
  };
  const std::array<cgpui::VulkanTextDrawCommand, 3> text_commands{
      cgpui::VulkanTextDrawCommand{.text_draw_index = 0},
      cgpui::VulkanTextDrawCommand{.text_draw_index = 0},
      cgpui::VulkanTextDrawCommand{.text_draw_index = 1},
  };

  cgpui::VulkanFrameDrawOrderCursor cursor(
      order, solid_draws, rounded_draws, text_commands);
  std::vector<cgpui::VulkanResolvedFrameDraw> resolved;
  while (const auto draw = cursor.next()) {
    resolved.push_back(*draw);
  }
  const std::array<cgpui::VulkanResolvedFrameDraw, 7> expected{
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::rounded_rect,
          .resource_index = 0,
      },
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::solid_rect,
          .resource_index = 0,
      },
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::text,
          .resource_index = 0,
      },
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::text,
          .resource_index = 1,
      },
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::text,
          .resource_index = 2,
      },
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::rounded_rect,
          .resource_index = 1,
      },
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::solid_rect,
          .resource_index = 1,
      },
  };
  if (resolved.size() != expected.size()) {
    return 10;
  }
  for (std::size_t index = 0; index < expected.size(); ++index) {
    if (resolved[index] != expected[index]) {
      return 11;
    }
  }
  return 0;
}

int test_text_draw_planning_retains_source_index() {
  const std::array<cgpui::VulkanGlyphAtlasDrawBinding, 1> bindings{
      cgpui::VulkanGlyphAtlasDrawBinding{
          .text_draw_index = 3,
          .page_index = 0,
          .first_quad_index = 0,
          .glyph_quad_count = 1,
          .descriptor_set = fake_handle<VkDescriptorSet>(1),
      },
  };
  const cgpui::VulkanTextPipelineResources pipeline{
      .layout = fake_handle<VkPipelineLayout>(2),
      .pipeline = fake_handle<VkPipeline>(3),
  };
  const cgpui::VulkanTextVertexBufferResources vertices{
      .buffer = fake_handle<VkBuffer>(4),
      .memory = fake_handle<VkDeviceMemory>(5),
      .vertex_count = 6,
      .byte_size = 6 * sizeof(cgpui::VulkanTextVertex),
  };
  const auto commands =
      cgpui::vulkan_plan_text_draw_commands(bindings, pipeline, vertices);
  return commands && commands->size() == 1 &&
                 commands->front().text_draw_index == 3
             ? 0
             : 20;
}

int test_structure_and_documentation() {
  const std::string order_header = read_source(
      "src/renderer/vulkan/vulkan_frame_draw_order_internal.hpp");
  const std::string order_source =
      read_source("src/renderer/vulkan/vulkan_frame_draw_order.cpp");
  const std::string recording_header = read_source(
      "src/renderer/vulkan/vulkan_frame_draw_recording_internal.hpp");
  const std::string recording_source =
      read_source("src/renderer/vulkan/vulkan_frame_draw_recording.cpp");
  const std::string renderer =
      read_source("src/renderer/vulkan/vulkan_renderer.cpp");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string command_recording =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string rounded_recording = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_draw_recording.cpp");
  const std::string text_recording =
      read_source("src/renderer/vulkan/vulkan_text_draw_recording.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (order_header.empty() || order_source.empty() || recording_header.empty() ||
      recording_source.empty() || renderer.empty() || presentation.empty() ||
      command_recording.empty() || rounded_recording.empty() ||
      text_recording.empty() || structure.empty()) {
    return 30;
  }
  if (!contains(order_header, "struct VulkanFrameDrawOrderEntry") ||
      !contains(order_header, "class VulkanFrameDrawOrderCursor") ||
      !contains(order_source, "VulkanFrameDrawOrderCursor::next(") ||
      contains(order_header, "std::vector") ||
      contains(order_source, "std::vector") ||
      !contains(renderer, "draw_order_") ||
      !contains(renderer, "append_draw(") ||
      !contains(presentation, "VulkanFrameDrawOrderEntry") ||
      !contains(command_recording, "vulkan_record_frame_draws(") ||
      contains(command_recording, "vulkan_record_rounded_rect_draws(") ||
      contains(command_recording, "vulkan_record_text_draws(") ||
      !contains(recording_source, "VulkanFrameDrawOrderCursor") ||
      !contains(recording_source, "vulkan_bind_rounded_rect_draw_state(") ||
      !contains(recording_source, "vulkan_record_rounded_rect_draw(") ||
      !contains(recording_source, "vulkan_bind_text_draw_state(") ||
      !contains(recording_source, "vulkan_record_text_draw(") ||
      !contains(text_recording, ".text_draw_index = binding.text_draw_index") ||
      !contains(structure, "vulkan_frame_draw_order.cpp") ||
      !contains(structure, "vulkan_frame_draw_recording.cpp")) {
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
      "Phase E Step 488",
      "VulkanFrameDrawOrderCursor",
      "stable authored interleaving",
      "Step 489",
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
  if (const int result = test_compact_cursor_preserves_authored_interleaving();
      result != 0) {
    return result;
  }
  if (const int result = test_text_draw_planning_retains_source_index();
      result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
