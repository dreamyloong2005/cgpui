#include "vulkan_present_pacing_internal.hpp"

#include <array>
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

int test_mailbox_low_latency_plan() {
  constexpr std::array modes{
      VK_PRESENT_MODE_FIFO_KHR,
      VK_PRESENT_MODE_FIFO_RELAXED_KHR,
      VK_PRESENT_MODE_MAILBOX_KHR,
  };
  const cgpui::VulkanPresentPacingPlan plan =
      cgpui::vulkan_plan_present_pacing(modes, 2, 0);
  if (plan.present_mode != VK_PRESENT_MODE_MAILBOX_KHR ||
      plan.swapchain_image_count != 3 || plan.max_frames_in_flight != 1 ||
      !plan.wait_for_previous_frame) {
    return 10;
  }
  return plan.fence_wait_timeout == std::numeric_limits<std::uint64_t>::max() &&
                 plan.image_acquire_timeout ==
                     std::numeric_limits<std::uint64_t>::max()
             ? 0
             : 11;
}

int test_fifo_fallback_and_bounded_depth() {
  constexpr std::array modes{
      VK_PRESENT_MODE_IMMEDIATE_KHR,
      VK_PRESENT_MODE_FIFO_KHR,
  };
  const cgpui::VulkanPresentPacingPlan plan =
      cgpui::vulkan_plan_present_pacing(modes, 2, 2);
  return plan.present_mode == VK_PRESENT_MODE_FIFO_KHR &&
                 plan.swapchain_image_count == 2 &&
                 plan.max_frames_in_flight == 1 &&
                 plan.wait_for_previous_frame
             ? 0
             : 20;
}

int test_image_depth_increment_saturates() {
  constexpr std::array modes{VK_PRESENT_MODE_FIFO_KHR};
  const cgpui::VulkanPresentPacingPlan plan =
      cgpui::vulkan_plan_present_pacing(
          modes, std::numeric_limits<std::uint32_t>::max(), 0);
  return plan.swapchain_image_count ==
                 std::numeric_limits<std::uint32_t>::max()
             ? 0
             : 30;
}

int test_structure_and_documentation() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_present_pacing_internal.hpp");
  const std::string policy =
      read_source("src/renderer/vulkan/vulkan_present_pacing.cpp");
  const std::string wait =
      read_source("src/renderer/vulkan/vulkan_present_pacing_wait.cpp");
  const std::string swapchain_header =
      read_source("src/renderer/vulkan/vulkan_swapchain_internal.hpp");
  const std::string create_info =
      read_source("src/renderer/vulkan/vulkan_swapchain_create_info.cpp");
  const std::string swapchain_create =
      read_source("src/renderer/vulkan/vulkan_swapchain_create.cpp");
  const std::string lifecycle =
      read_source("src/renderer/vulkan/vulkan_swapchain_lifecycle.cpp");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string state =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (header.empty() || policy.empty() || wait.empty() ||
      swapchain_header.empty() ||
      create_info.empty() || swapchain_create.empty() || lifecycle.empty() ||
      presentation.empty() || state.empty() || structure.empty()) {
    return 40;
  }
  if (!contains(header, "struct VulkanPresentPacingPlan") ||
      !contains(policy, "vulkan_plan_present_pacing(") ||
      !contains(policy, "VK_PRESENT_MODE_MAILBOX_KHR") ||
      !contains(policy, "VK_PRESENT_MODE_FIFO_KHR") ||
      !contains(wait, "VulkanRendererState::wait_for_present_pacing()") ||
      !contains(wait, "vkWaitForFences") ||
      !contains(swapchain_header, "VulkanPresentPacingPlan present_pacing") ||
      !contains(create_info, "vulkan_plan_present_pacing(") ||
      !contains(swapchain_create, ".present_pacing = plan.present_pacing") ||
      !contains(lifecycle, "present_pacing_ = resources.present_pacing") ||
      !contains(presentation, "wait_for_present_pacing()") ||
      !contains(presentation, "present_pacing_.image_acquire_timeout") ||
      !contains(state, "VulkanPresentPacingPlan present_pacing_") ||
      !contains(structure, "vulkan_present_pacing.cpp") ||
      contains(policy, "vkWaitForFences") ||
      contains(policy, "vkAcquireNextImageKHR") ||
      contains(policy, "vkQueuePresentKHR") ||
      contains(wait, "vkAcquireNextImageKHR") ||
      contains(wait, "vkQueuePresentKHR")) {
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
      "Phase E Step 512",
      "focused Vulkan present pacing policy",
      "MAILBOX with FIFO fallback",
      "saturation-safe swapchain image depth",
      "one CPU frame in flight",
      "Step 513 next-frame scheduling",
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
  if (const int result = test_mailbox_low_latency_plan(); result != 0) {
    return result;
  }
  if (const int result = test_fifo_fallback_and_bounded_depth(); result != 0) {
    return result;
  }
  if (const int result = test_image_depth_increment_saturates(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
