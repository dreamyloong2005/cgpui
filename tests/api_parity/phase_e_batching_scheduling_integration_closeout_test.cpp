#include <array>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = CGPUI_SOURCE_ROOT; root != nullptr) {
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

struct SourceEvidence {
  const char* path;
  const char* needle;
};

} // namespace

int main() {
  const std::string manifests = read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/renderer/mcpp.toml") +
      read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  if (manifests.empty() ||
      roadmap.empty() ||
      ledger_md.empty() ||
      ledger_json.empty()) {
    return 1;
  }

  if (!contains(
          manifests,
          "\"phase_e_batching_scheduling_integration_closeout_test.cpp\"") ||
      !contains(
          manifests,
          "\"phase_e_batching_scheduling_integration_closeout_test.cpp\"")) {
    return 2;
  }

  constexpr std::array target_evidence{
      "\"vulkan_frame_geometry_buffer_test.cpp\"",
      "\"vulkan_frame_command_reuse_test.cpp\"",
      "\"vulkan_pipeline_switch_batching_test.cpp\"",
      "\"vulkan_upload_barrier_batching_test.cpp\"",
      "\"vulkan_swapchain_recovery_test.cpp\"",
      "\"vulkan_present_pacing_test.cpp\"",
      "\"window_runtime_frame_scheduling_test.cpp\"",
      "\"renderer_source_structure_test.cpp\"",
      "\"ui_source_structure_test.cpp\"",
  };
  for (std::size_t index = 0; index < target_evidence.size(); ++index) {
    if (!contains(manifests, target_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array module_evidence{
      SourceEvidence{
          "src/renderer/vulkan/vulkan_frame_geometry_buffer_internal.hpp",
          "struct VulkanFrameGeometryBufferResources"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_frame_command_reuse_internal.hpp",
          "struct VulkanFrameCommandReuseState"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_frame_pipeline_switch_internal.hpp",
          "struct VulkanFramePipelineSwitchPlan"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_upload_barrier_batch_internal.hpp",
          "struct VulkanUploadBarrierPlan"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_swapchain_recovery_policy_internal.hpp",
          "struct VulkanSwapchainRecoveryPlan"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_present_pacing_internal.hpp",
          "struct VulkanPresentPacingPlan"},
      SourceEvidence{
          "src/ui/runtime_frame_scheduling.cpp",
          "WindowRuntime::complete_frame_scheduling()"},
  };
  for (std::size_t index = 0; index < module_evidence.size(); ++index) {
    const std::string source = read_source(module_evidence[index].path);
    if (source.empty() || !contains(source, module_evidence[index].needle)) {
      return 30 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      SourceEvidence{
          "tests/renderer/vulkan_frame_geometry_buffer_test.cpp",
          "test_matching_capacity_is_reused"},
      SourceEvidence{
          "tests/renderer/vulkan_frame_command_reuse_test.cpp",
          "test_first_frame_records_then_exact_signature_reuses"},
      SourceEvidence{
          "tests/renderer/vulkan_pipeline_switch_batching_test.cpp",
          "test_shared_rounded_rect_pipeline_is_coalesced"},
      SourceEvidence{
          "tests/renderer/vulkan_upload_barrier_batching_test.cpp",
          "test_unique_targets_share_one_ordered_wave"},
      SourceEvidence{
          "tests/renderer/vulkan_swapchain_recovery_test.cpp",
          "test_present_result_policy"},
      SourceEvidence{
          "tests/renderer/vulkan_present_pacing_test.cpp",
          "test_mailbox_low_latency_plan"},
      SourceEvidence{
          "tests/ui/window_runtime_frame_scheduling_test.cpp",
          "test_render_time_invalidations_schedule_one_next_frame"},
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    const std::string source = read_source(behavior_evidence[index].path);
    if (source.empty() || !contains(source, behavior_evidence[index].needle)) {
      return 50 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "batching and frame scheduling integration closeout",
      "reusable geometry buffers through next-frame scheduling"};
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
      !contains(ledger_md, required_docs[index]) ||
      !contains(ledger_json, required_docs[index])) {
      return 70 + static_cast<int>(index);
    }
  }

  return 0;
}
