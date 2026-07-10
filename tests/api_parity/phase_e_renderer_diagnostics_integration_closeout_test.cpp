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
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
  }
  return source ? std::string{
                      std::istreambuf_iterator<char>(source),
                      std::istreambuf_iterator<char>()}
                : std::string{};
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
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  if (xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty()) {
    return 1;
  }

  if (!contains(
          xmake,
          "target(\"phase_e_renderer_diagnostics_integration_closeout_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/"
          "phase_e_renderer_diagnostics_integration_closeout_test.cpp")) {
    return 2;
  }

  constexpr std::array target_evidence{
      "target(\"renderer_frame_diagnostics_test\")",
      "target(\"renderer_upload_byte_diagnostics_test\")",
      "target(\"renderer_draw_count_diagnostics_test\")",
      "target(\"renderer_dropped_resource_diagnostics_test\")",
      "target(\"renderer_frame_timing_diagnostics_test\")",
      "target(\"vulkan_frame_diagnostic_snapshot_test\")",
      "target(\"runtime_renderer_diagnostics_test\")",
      "target(\"renderer_source_structure_test\")",
      "target(\"ui_source_structure_test\")",
  };
  for (std::size_t index = 0; index < target_evidence.size(); ++index) {
    if (!contains(xmake, target_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array module_evidence{
      SourceEvidence{
          "src/renderer/renderer_frame_diagnostics.cpp",
          "compare_renderer_frame_work("},
      SourceEvidence{
          "src/renderer/renderer_frame_upload_diagnostics.cpp",
          "renderer_upload_byte_counts("},
      SourceEvidence{
          "src/renderer/renderer_frame_draw_diagnostics.cpp",
          "renderer_add_draw_count("},
      SourceEvidence{
          "src/renderer/renderer_dropped_resource_diagnostics.cpp",
          "classify_renderer_dropped_resources("},
      SourceEvidence{
          "src/renderer/renderer_frame_timing_diagnostics.cpp",
          "compare_renderer_frame_timing("},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_frame_diagnostic_snapshot.cpp",
          "vulkan_build_frame_diagnostic_snapshot("},
      SourceEvidence{
          "src/ui/runtime_renderer_diagnostics.cpp",
          "apply_runtime_renderer_frame_diagnostics("},
  };
  for (std::size_t index = 0; index < module_evidence.size(); ++index) {
    const std::string source = read_source(module_evidence[index].path);
    if (source.empty() || !contains(source, module_evidence[index].needle)) {
      return 30 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      SourceEvidence{
          "tests/renderer/renderer_frame_diagnostics_test.cpp",
          "test_exact_work_matches"},
      SourceEvidence{
          "tests/renderer/renderer_upload_byte_diagnostics_test.cpp",
          "test_counts_glyph_and_image_payload_bytes"},
      SourceEvidence{
          "tests/renderer/renderer_draw_count_diagnostics_test.cpp",
          "test_counts_every_primitive_kind"},
      SourceEvidence{
          "tests/renderer/renderer_dropped_resource_diagnostics_test.cpp",
          "test_unsupported_resources_are_classified"},
      SourceEvidence{
          "tests/renderer/renderer_frame_timing_diagnostics_test.cpp",
          "test_counts_every_frame_stage"},
      SourceEvidence{
          "tests/renderer/vulkan_frame_diagnostic_snapshot_test.cpp",
          "test_builds_frame_snapshot"},
      SourceEvidence{
          "tests/ui/runtime_renderer_diagnostics_test.cpp",
          "test_propagates_renderer_diagnostics_after_present"},
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    const std::string source = read_source(behavior_evidence[index].path);
    if (source.empty() || !contains(source, behavior_evidence[index].needle)) {
      return 50 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase E Step 522",
      "renderer diagnostics integration closeout",
      "Steps 515-521",
      "work through runtime propagation",
      "Step 523 pixel/screenshot testing",
  };
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
        !contains(ledger_md, required_docs[index]) ||
        !contains(ledger_json, required_docs[index]) ||
        !contains(task_plan, required_docs[index]) ||
        !contains(findings, required_docs[index])) {
      return 70 + static_cast<int>(index);
    }
  }

  if (!contains(
          roadmap,
          "- [x] Phase E Step 522 closes the renderer diagnostics") ||
      contains(roadmap, "- [ ] Phase E Step 522:")) {
    return 80;
  }
  return 0;
}
