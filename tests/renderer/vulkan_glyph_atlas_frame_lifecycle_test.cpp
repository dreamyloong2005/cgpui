#include "cgpui/renderer/renderer.hpp"

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

int test_acquired_command_buffer_recording_order() {
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string recovery =
      read_source("src/renderer/vulkan/vulkan_presentation_recovery.cpp");
  const std::string submission =
      read_source("src/renderer/vulkan/vulkan_frame_submission.cpp");
  const std::string state =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  if (presentation.empty() || recovery.empty() || submission.empty() ||
      state.empty()) {
    return 10;
  }

  const std::size_t acquire = presentation.find("vkAcquireNextImageKHR");
  const std::size_t record =
      presentation.find("record_vulkan_frame_command_buffer(");
  const std::size_t submit =
      presentation.find("submit_frame(command_buffer)");
  if (acquire == std::string::npos || record == std::string::npos ||
      submit == std::string::npos || !(acquire < record && record < submit)) {
    return 11;
  }
  const std::size_t reset_fence = submission.find("vkResetFences");
  const std::size_t queue_submit = submission.find("vkQueueSubmit");
  if (reset_fence == std::string::npos || queue_submit == std::string::npos ||
      reset_fence >= queue_submit || contains(presentation, "vkResetFences") ||
      contains(presentation, "vkQueueSubmit")) {
    return 12;
  }
  if (!contains(presentation, "command_buffers_[image_index]") ||
      contains(presentation,
               "image_index < command_buffers_.size()") ||
      !contains(presentation, "recover_after_failed_record(") ||
      !contains(recovery,
                "VulkanRendererState::recover_after_failed_record(") ||
      !contains(state, "recover_after_failed_record(std::string message)")) {
    return 13;
  }
  return 0;
}

int test_multi_frame_smoke_structure() {
  const std::string lifetime =
      read_source("tests/renderer/vulkan_frame_lifetime_test.cpp");
  if (lifetime.empty()) {
    return 20;
  }
  return contains(lifetime, "present_text_frame(") &&
                 contains(lifetime, "\"ab\"") &&
                 contains(lifetime, "\"abc\"") &&
                 contains(lifetime, "test_incremental_glyph_atlas_frames(")
             ? 0
             : 21;
}

int test_step_462_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 462",
      "acquired command buffer",
      "recover_after_failed_record",
      "ab -> ab -> abc",
      "Step 463",
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
  if (const int result = test_acquired_command_buffer_recording_order();
      result != 0) {
    return result;
  }
  if (const int result = test_multi_frame_smoke_structure(); result != 0) {
    return result;
  }
  return test_step_462_documentation();
}
