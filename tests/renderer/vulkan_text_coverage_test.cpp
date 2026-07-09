#include "vulkan_text_coverage_internal.hpp"
#include "vulkan_text_pipeline_internal.hpp"

#include <cmath>
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

bool approximately_equal(float left, float right) {
  return std::abs(left - right) < 0.0001F;
}

int test_coverage_policy_contract() {
  using enum cgpui::VulkanTextAlphaMode;
  using enum cgpui::VulkanTextCoverageTransfer;
  const cgpui::VulkanTextCoveragePolicy policy =
      cgpui::vulkan_default_text_coverage_policy();
  if (policy.transfer != linear || policy.gamma != 1.0F ||
      policy.alpha_mode != straight_color_coverage_alpha) {
    return 10;
  }
  return 0;
}

int test_coverage_transfer_and_alpha() {
  const cgpui::VulkanTextCoveragePolicy gamma_policy{
      .transfer = cgpui::VulkanTextCoverageTransfer::power,
      .gamma = 2.0F,
  };
  if (!approximately_equal(
          cgpui::vulkan_resolve_text_coverage(-1.0F, gamma_policy), 0.0F) ||
      !approximately_equal(
          cgpui::vulkan_resolve_text_coverage(0.5F, gamma_policy), 0.25F) ||
      !approximately_equal(
          cgpui::vulkan_resolve_text_coverage(2.0F, gamma_policy), 1.0F)) {
    return 20;
  }

  const cgpui::Color resolved = cgpui::vulkan_apply_text_coverage(
      cgpui::Color{.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 0.8F},
      0.5F,
      gamma_policy);
  if (!approximately_equal(resolved.r, 0.2F) ||
      !approximately_equal(resolved.g, 0.4F) ||
      !approximately_equal(resolved.b, 0.6F) ||
      !approximately_equal(resolved.a, 0.2F)) {
    return 21;
  }
  return 0;
}

int test_shader_and_blend_contract() {
  const std::string fragment =
      read_source("src/renderer/vulkan/shaders/text.frag.glsl");
  if (fragment.empty() || !contains(fragment, "coverage_gamma") ||
      !contains(fragment, "pow(") || !contains(fragment, "clamp(") ||
      !contains(fragment, "in_color.a * coverage")) {
    return 30;
  }
  const VkPipelineColorBlendAttachmentState blend =
      cgpui::vulkan_text_pipeline_blend_attachment_state();
  if (blend.srcColorBlendFactor != VK_BLEND_FACTOR_SRC_ALPHA ||
      blend.dstColorBlendFactor != VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ||
      blend.srcAlphaBlendFactor != VK_BLEND_FACTOR_ONE ||
      blend.dstAlphaBlendFactor != VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA) {
    return 31;
  }
  return 0;
}

int test_coverage_module_structure() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_text_coverage_internal.hpp");
  const std::string source =
      read_source("src/renderer/vulkan/vulkan_text_coverage.cpp");
  const std::string binaries =
      read_source("src/renderer/vulkan/vulkan_text_shader_binaries.cpp");
  if (header.empty() || source.empty() || binaries.empty()) {
    return 40;
  }
  if (!contains(header, "struct VulkanTextCoveragePolicy") ||
      !contains(source, "vulkan_resolve_text_coverage(") ||
      !contains(source, "std::pow") ||
      !contains(binaries, "vulkan_text_fragment_shader_spirv(")) {
    return 41;
  }
  return 0;
}

int test_step_473_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 473",
      "VulkanTextCoveragePolicy",
      "straight_color_coverage_alpha",
      "Step 474",
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
  if (const int result = test_coverage_policy_contract(); result != 0) {
    return result;
  }
  if (const int result = test_coverage_transfer_and_alpha(); result != 0) {
    return result;
  }
  if (const int result = test_shader_and_blend_contract(); result != 0) {
    return result;
  }
  if (const int result = test_coverage_module_structure(); result != 0) {
    return result;
  }
  return test_step_473_documentation();
}
