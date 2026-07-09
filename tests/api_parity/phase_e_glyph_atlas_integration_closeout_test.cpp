#include <array>
#include <cstddef>
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
  const std::string root = source_root();
  std::ifstream source(root + "/" + path);
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

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string production_header =
      read_source("include/cgpui/renderer/glyph_atlas_production.hpp");
  const std::string resources_header = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_resources_internal.hpp");
  const std::string uploads_header = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_uploads_internal.hpp");
  const std::string draw_header = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_draw_bindings_internal.hpp");
  const std::string resources =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_resources.cpp");
  const std::string staging =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_staging.cpp");
  const std::string upload_recording = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_upload_recording.cpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_frame.cpp");
  const std::string draw_bindings = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_draw_bindings.cpp");
  const std::string draw_data =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_draw_data.cpp");
  const std::string command_recording =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string multi_page_test =
      read_source("tests/renderer/vulkan_glyph_atlas_multi_page_test.cpp");
  const std::string lifecycle_test = read_source(
      "tests/renderer/vulkan_glyph_atlas_frame_lifecycle_test.cpp");
  const std::string draw_binding_test = read_source(
      "tests/renderer/vulkan_glyph_atlas_draw_binding_test.cpp");
  const std::string draw_data_test = read_source(
      "tests/renderer/vulkan_glyph_atlas_draw_data_test.cpp");
  const std::string lifetime_test =
      read_source("tests/renderer/vulkan_frame_lifetime_test.cpp");

  if (xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty() ||
      production_header.empty() || resources_header.empty() ||
      uploads_header.empty() || draw_header.empty() || resources.empty() ||
      staging.empty() || upload_recording.empty() || frame.empty() ||
      draw_bindings.empty() || draw_data.empty() || command_recording.empty() ||
      multi_page_test.empty() || lifecycle_test.empty() ||
      draw_binding_test.empty() || draw_data_test.empty() ||
      lifetime_test.empty()) {
    return 1;
  }

  if (!contains(
          xmake,
          "target(\"phase_e_glyph_atlas_integration_closeout_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/"
          "phase_e_glyph_atlas_integration_closeout_test.cpp")) {
    return 2;
  }

  constexpr std::array target_evidence{
      "target(\"phase_e_glyph_atlas_production_test\")",
      "target(\"vulkan_glyph_atlas_descriptor_test\")",
      "target(\"vulkan_glyph_atlas_upload_test\")",
      "target(\"vulkan_glyph_atlas_multi_page_test\")",
      "target(\"vulkan_glyph_atlas_frame_lifecycle_test\")",
      "target(\"vulkan_glyph_atlas_draw_binding_test\")",
      "target(\"vulkan_glyph_atlas_draw_data_test\")",
  };
  for (std::size_t index = 0; index < target_evidence.size(); ++index) {
    if (!contains(xmake, target_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase E Step 466 glyph atlas integration closeout",
      "tests/api_parity/phase_e_glyph_atlas_integration_closeout_test.cpp",
      "Steps 459-465",
      "alpha8 page resources",
      "descriptor capacity",
      "dirty uploads",
      "acquired command buffer",
      "three atlas pages",
      "VulkanGlyphAtlasDrawBinding",
      "VulkanGlyphAtlasDrawData",
      "contiguous page runs",
      "Step 467 text shader pipeline",
  };
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
        !contains(ledger_md, required_docs[index]) ||
        !contains(ledger_json, required_docs[index]) ||
        !contains(task_plan, required_docs[index]) ||
        !contains(findings, required_docs[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  if (!contains(roadmap,
                "- [x] Phase E Step 466 closes the production glyph atlas") ||
      contains(roadmap, "- [ ] Step 466: Close the production glyph atlas")) {
    return 50;
  }

  constexpr std::array module_evidence{
      "struct GlyphAtlasProductionPlan",
      "class GlyphAtlasProductionResourceState",
      "vulkan_glyph_atlas_descriptor_capacity = 256",
      "struct VulkanGlyphAtlasPageResource",
      "struct VulkanGlyphAtlasStagingUpload",
      "struct VulkanGlyphAtlasUploadResources",
      "struct VulkanGlyphAtlasDrawBinding",
      "struct VulkanGlyphAtlasDrawData",
  };
  const std::array module_sources{
      &production_header,
      &production_header,
      &resources_header,
      &resources_header,
      &uploads_header,
      &uploads_header,
      &draw_header,
      &draw_header,
  };
  for (std::size_t index = 0; index < module_evidence.size(); ++index) {
    if (!contains(*module_sources[index], module_evidence[index])) {
      return 60 + static_cast<int>(index);
    }
  }

  if (!contains(resources, "vulkan_glyph_atlas_plan_fits_descriptor_capacity(") ||
      !contains(staging, "vulkan_stage_glyph_atlas_uploads(") ||
      !contains(upload_recording, "vkCmdCopyBufferToImage(") ||
      !contains(frame, "prepare_glyph_atlas_frame(") ||
      !contains(frame, "vulkan_resolve_glyph_atlas_draw_bindings(") ||
      !contains(draw_bindings,
                "vulkan_validate_glyph_atlas_draw_bindings(") ||
      !contains(draw_data, "vulkan_plan_glyph_atlas_draw_data(") ||
      !contains(command_recording,
                "vulkan_validate_glyph_atlas_draw_bindings(")) {
    return 80;
  }

  const std::size_t validate_index = command_recording.find(
      "vulkan_validate_glyph_atlas_draw_bindings(");
  const std::size_t render_pass_index =
      command_recording.find("vkCmdBeginRenderPass");
  if (validate_index == std::string::npos ||
      render_pass_index == std::string::npos ||
      validate_index >= render_pass_index) {
    return 81;
  }

  constexpr std::array behavior_evidence{
      "test_three_page_allocation_and_upload_planning",
      "test_descriptor_capacity_rejection_is_preflight",
      "test_acquired_command_buffer_recording_order",
      "test_page_usage_plans_and_resolves_descriptor_bindings",
      "test_draw_data_preserves_contiguous_page_runs",
      "test_multi_page_glyph_atlas_frame",
  };
  const std::array behavior_sources{
      &multi_page_test,
      &multi_page_test,
      &lifecycle_test,
      &draw_binding_test,
      &draw_data_test,
      &lifetime_test,
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    if (!contains(*behavior_sources[index], behavior_evidence[index])) {
      return 90 + static_cast<int>(index);
    }
  }

  return 0;
}
