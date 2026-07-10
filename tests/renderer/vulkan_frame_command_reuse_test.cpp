#include "vulkan_frame_command_reuse_internal.hpp"

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

struct Fixture {
  std::array<cgpui::VulkanRoundedRectDrawRange, 1> solid_draws{
      cgpui::VulkanRoundedRectDrawRange{
          .source_index = 0,
          .vertex_count = 5,
          .index_count = 12,
          .clip_rect = cgpui::Rect{.size = {.width = 20.0F, .height = 10.0F}},
      },
  };
  std::array<cgpui::VulkanRoundedRectDrawRange, 1> rounded_draws{
      cgpui::VulkanRoundedRectDrawRange{
          .source_index = 1,
          .vertex_count = 5,
          .index_count = 12,
      },
  };
  std::array<cgpui::VulkanTextDrawCommand, 1> text_commands{
      cgpui::VulkanTextDrawCommand{
          .text_draw_index = 2,
          .descriptor_set = fake_handle<VkDescriptorSet>(20),
          .vertex_count = 6,
      },
  };
  std::array<cgpui::VulkanImageDrawCommand, 1> image_commands{
      cgpui::VulkanImageDrawCommand{
          .image_draw_index = 3,
          .descriptor_set = fake_handle<VkDescriptorSet>(21),
          .vertex_count = 6,
          .clip_stack =
              cgpui::RendererClipStackRecord{
                  .clips = {
                      cgpui::Rect{
                          .origin = {.x = 1.0F, .y = 2.0F},
                          .size = {.width = 30.0F, .height = 40.0F},
                      },
                  },
                  .full_depth = 1,
              },
      },
  };
  std::array<cgpui::VulkanFrameDrawOrderEntry, 4> draw_order{
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
          .command_index = 0,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::rounded_rect,
          .command_index = 0,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::text,
          .command_index = 0,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::image,
          .command_index = 0,
      },
  };

  cgpui::VulkanFrameCommandSignatureView view() const {
    return cgpui::VulkanFrameCommandSignatureView{
        .render_pass = fake_handle<VkRenderPass>(1),
        .framebuffer = fake_handle<VkFramebuffer>(2),
        .extent = VkExtent2D{.width = 800, .height = 600},
        .clear_color = cgpui::Color{.r = 0.1F, .g = 0.2F, .b = 0.3F, .a = 1.0F},
        .rounded_rect_pipeline_layout = fake_handle<VkPipelineLayout>(3),
        .rounded_rect_pipeline = fake_handle<VkPipeline>(4),
        .text_pipeline_layout = fake_handle<VkPipelineLayout>(5),
        .text_pipeline = fake_handle<VkPipeline>(6),
        .image_pipeline_layout = fake_handle<VkPipelineLayout>(7),
        .image_pipeline = fake_handle<VkPipeline>(8),
        .solid_vertex_buffer = fake_handle<VkBuffer>(9),
        .solid_index_buffer = fake_handle<VkBuffer>(10),
        .rounded_vertex_buffer = fake_handle<VkBuffer>(11),
        .rounded_index_buffer = fake_handle<VkBuffer>(12),
        .text_vertex_buffer = fake_handle<VkBuffer>(13),
        .image_vertex_buffer = fake_handle<VkBuffer>(14),
        .solid_draws = solid_draws,
        .rounded_draws = rounded_draws,
        .text_commands = text_commands,
        .image_commands = image_commands,
        .draw_order = draw_order,
    };
  }
};

int test_first_frame_records_then_exact_signature_reuses() {
  Fixture fixture;
  cgpui::VulkanFrameCommandReuseState state;
  const auto first = cgpui::vulkan_plan_frame_command_reuse(
      state, fixture.view(), false);
  if (first.action != cgpui::VulkanFrameCommandReuseAction::record ||
      first.reason != cgpui::VulkanFrameCommandReuseReason::no_recording) {
    return 10;
  }
  cgpui::vulkan_commit_frame_command_recording(
      state, fixture.view(), false);
  const auto second = cgpui::vulkan_plan_frame_command_reuse(
      state, fixture.view(), false);
  if (second.action != cgpui::VulkanFrameCommandReuseAction::reuse ||
      second.reason != cgpui::VulkanFrameCommandReuseReason::exact_match) {
    return 11;
  }
  cgpui::vulkan_commit_frame_command_reuse(state);
  return state.recording_count == 1 && state.reuse_count == 1 ? 0 : 12;
}

int test_pending_uploads_force_record_and_invalidate() {
  Fixture fixture;
  cgpui::VulkanFrameCommandReuseState state;
  cgpui::vulkan_commit_frame_command_recording(
      state, fixture.view(), false);
  const auto plan = cgpui::vulkan_plan_frame_command_reuse(
      state, fixture.view(), true);
  if (plan.action != cgpui::VulkanFrameCommandReuseAction::record ||
      plan.reason != cgpui::VulkanFrameCommandReuseReason::pending_uploads) {
    return 20;
  }
  cgpui::vulkan_commit_frame_command_recording(state, fixture.view(), true);
  return !state.valid && state.recording_count == 2 ? 0 : 21;
}

int test_semantic_changes_force_record() {
  Fixture fixture;
  cgpui::VulkanFrameCommandReuseState state;
  cgpui::vulkan_commit_frame_command_recording(
      state, fixture.view(), false);

  auto changed_clear = fixture.view();
  changed_clear.clear_color.r = 0.9F;
  if (cgpui::vulkan_plan_frame_command_reuse(
          state, changed_clear, false)
          .reason != cgpui::VulkanFrameCommandReuseReason::signature_changed) {
    return 30;
  }
  fixture.draw_order[0].command_index = 1;
  if (cgpui::vulkan_plan_frame_command_reuse(
          state, fixture.view(), false)
          .reason != cgpui::VulkanFrameCommandReuseReason::signature_changed) {
    return 31;
  }
  fixture.draw_order[0].command_index = 0;
  fixture.text_commands[0].descriptor_set = fake_handle<VkDescriptorSet>(99);
  if (cgpui::vulkan_plan_frame_command_reuse(
          state, fixture.view(), false)
          .reason != cgpui::VulkanFrameCommandReuseReason::signature_changed) {
    return 32;
  }
  fixture.text_commands[0].descriptor_set = fake_handle<VkDescriptorSet>(20);
  fixture.image_commands[0].clip_stack.clips[0].origin.x = 50.0F;
  return cgpui::vulkan_plan_frame_command_reuse(
             state, fixture.view(), false)
                 .reason ==
             cgpui::VulkanFrameCommandReuseReason::signature_changed
             ? 0
             : 33;
}

int test_bulk_invalidation_retains_counters() {
  Fixture fixture;
  std::array<cgpui::VulkanFrameCommandReuseState, 2> states;
  for (auto& state : states) {
    cgpui::vulkan_commit_frame_command_recording(
        state, fixture.view(), false);
  }
  cgpui::vulkan_invalidate_frame_command_reuse(states);
  return !states[0].valid && !states[1].valid &&
                 states[0].recording_count == 1 &&
                 states[1].recording_count == 1
             ? 0
             : 40;
}

int test_structure_and_documentation() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_frame_command_reuse_internal.hpp");
  const std::string source = read_source(
      "src/renderer/vulkan/vulkan_frame_command_reuse.cpp");
  const std::string command = read_source(
      "src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string presentation = read_source(
      "src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string state = read_source(
      "src/renderer/vulkan/vulkan_state_internal.hpp");
  const std::string lifecycle = read_source(
      "src/renderer/vulkan/vulkan_swapchain_lifecycle.cpp");
  const std::string recovery = read_source(
      "src/renderer/vulkan/vulkan_presentation_recovery.cpp");
  const std::string structure = read_source(
      "tests/architecture/renderer_source_structure_test.cpp");
  if (header.empty() || source.empty() || command.empty() ||
      presentation.empty() || state.empty() || lifecycle.empty() ||
      recovery.empty() || structure.empty()) {
    return 50;
  }
  const std::size_t plan =
      command.find("vulkan_plan_frame_command_reuse(");
  const std::size_t reset = command.find("vkResetCommandBuffer");
  if (!contains(header, "struct VulkanFrameCommandReuseState") ||
      !contains(source, "vulkan_commit_frame_command_recording(") ||
      plan == std::string::npos || reset == std::string::npos ||
      !(plan < reset) ||
      contains(command, "VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT") ||
      !contains(presentation, "command_reuse_states_[image_index]") ||
      !contains(state,
                "std::vector<VulkanFrameCommandReuseState> command_reuse_states_") ||
      !contains(lifecycle, "command_reuse_states_.clear()") ||
      !contains(recovery,
                "vulkan_invalidate_frame_command_reuse(command_reuse_states_)") ||
      !contains(structure, "vulkan_frame_command_reuse.cpp")) {
    return 51;
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
      "Phase E Step 508",
      "exact semantic command signature",
      "per-swapchain-image recorded command reuse",
      "pending uploads force recording",
      "Step 509 pipeline-switch batching",
  };
  for (const char* value : required) {
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
  if (const int result = test_first_frame_records_then_exact_signature_reuses();
      result != 0) {
    return result;
  }
  if (const int result = test_pending_uploads_force_record_and_invalidate();
      result != 0) {
    return result;
  }
  if (const int result = test_semantic_changes_force_record(); result != 0) {
    return result;
  }
  if (const int result = test_bulk_invalidation_retains_counters(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
