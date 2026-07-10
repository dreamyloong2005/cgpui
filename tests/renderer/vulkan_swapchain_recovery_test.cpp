#include "vulkan_swapchain_recovery_policy_internal.hpp"

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

int test_acquire_result_policy() {
  const auto ready = cgpui::vulkan_plan_swapchain_recovery(
      cgpui::VulkanSwapchainOperation::acquire, VK_SUCCESS);
  const auto suboptimal = cgpui::vulkan_plan_swapchain_recovery(
      cgpui::VulkanSwapchainOperation::acquire, VK_SUBOPTIMAL_KHR);
  const auto out_of_date = cgpui::vulkan_plan_swapchain_recovery(
      cgpui::VulkanSwapchainOperation::acquire, VK_ERROR_OUT_OF_DATE_KHR);
  const auto failed = cgpui::vulkan_plan_swapchain_recovery(
      cgpui::VulkanSwapchainOperation::acquire,
      VK_ERROR_SURFACE_LOST_KHR);

  if (!ready.operation_succeeded || !ready.proceed_with_frame ||
      ready.recovery_timing != cgpui::VulkanSwapchainRecoveryTiming::none) {
    return 10;
  }
  if (!suboptimal.operation_succeeded || !suboptimal.proceed_with_frame ||
      suboptimal.recovery_timing !=
          cgpui::VulkanSwapchainRecoveryTiming::after_frame ||
      suboptimal.retry_frame) {
    return 11;
  }
  if (out_of_date.operation_succeeded || out_of_date.proceed_with_frame ||
      out_of_date.recovery_timing !=
          cgpui::VulkanSwapchainRecoveryTiming::before_frame ||
      !out_of_date.retry_frame) {
    return 12;
  }
  return !failed.operation_succeeded && !failed.proceed_with_frame &&
                 failed.recovery_timing ==
                     cgpui::VulkanSwapchainRecoveryTiming::none &&
                 !failed.retry_frame
             ? 0
             : 13;
}

int test_present_result_policy() {
  const auto ready = cgpui::vulkan_plan_swapchain_recovery(
      cgpui::VulkanSwapchainOperation::present, VK_SUCCESS);
  const auto suboptimal = cgpui::vulkan_plan_swapchain_recovery(
      cgpui::VulkanSwapchainOperation::present, VK_SUBOPTIMAL_KHR);
  const auto out_of_date = cgpui::vulkan_plan_swapchain_recovery(
      cgpui::VulkanSwapchainOperation::present, VK_ERROR_OUT_OF_DATE_KHR);

  if (!ready.operation_succeeded ||
      ready.recovery_timing != cgpui::VulkanSwapchainRecoveryTiming::none) {
    return 20;
  }
  if (!suboptimal.operation_succeeded ||
      suboptimal.recovery_timing !=
          cgpui::VulkanSwapchainRecoveryTiming::after_frame ||
      suboptimal.retry_frame) {
    return 21;
  }
  return !out_of_date.operation_succeeded &&
                 out_of_date.recovery_timing ==
                     cgpui::VulkanSwapchainRecoveryTiming::after_frame &&
                 out_of_date.retry_frame
             ? 0
             : 22;
}

int test_structure_and_documentation() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_swapchain_recovery_policy_internal.hpp");
  const std::string policy = read_source(
      "src/renderer/vulkan/vulkan_swapchain_recovery_policy.cpp");
  const std::string recovery = read_source(
      "src/renderer/vulkan/vulkan_swapchain_recovery.cpp");
  const std::string presentation = read_source(
      "src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string state = read_source(
      "src/renderer/vulkan/vulkan_state_internal.hpp");
  const std::string structure = read_source(
      "tests/architecture/renderer_source_structure_test.cpp");
  if (header.empty() || policy.empty() || recovery.empty() ||
      presentation.empty() || state.empty() || structure.empty()) {
    return 30;
  }
  if (!contains(header, "struct VulkanSwapchainRecoveryPlan") ||
      !contains(policy, "vulkan_plan_swapchain_recovery(") ||
      !contains(recovery,
                "VulkanRendererState::recover_swapchain_after_surface_status(") ||
      !contains(recovery, "resize(descriptor_.framebuffer_size") ||
      !contains(presentation, "vulkan_plan_swapchain_recovery(") ||
      !contains(state, "recover_swapchain_after_surface_status()") ||
      !contains(structure, "vulkan_swapchain_recovery_policy.cpp") ||
      contains(policy, "vkAcquireNextImageKHR") ||
      contains(policy, "vkQueuePresentKHR")) {
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
      "Phase E Step 511",
      "automatic swapchain recreation",
      "out-of-date results return a retryable frame error",
      "suboptimal frames recreate after submission",
      "presentation remains unblocked after successful recovery",
      "Step 512 present pacing",
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
  if (const int result = test_acquire_result_policy(); result != 0) {
    return result;
  }
  if (const int result = test_present_result_policy(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
