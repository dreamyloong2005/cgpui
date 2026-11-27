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
          "\"phase_e_renderer_diagnostics_integration_closeout_test.cpp\"") ||
      !contains(
          manifests,
          "\"phase_e_renderer_diagnostics_integration_closeout_test.cpp\"")) {
    return 2;
  }

  constexpr std::array target_evidence{
      "\"renderer_frame_diagnostics_test.cpp\"",
      "\"renderer_upload_byte_diagnostics_test.cpp\"",
      "\"renderer_draw_count_diagnostics_test.cpp\"",
      "\"renderer_dropped_resource_diagnostics_test.cpp\"",
      "\"renderer_frame_timing_diagnostics_test.cpp\"",
      "\"vulkan_frame_diagnostic_snapshot_test.cpp\"",
      "\"runtime_renderer_diagnostics_test.cpp\"",
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
      "renderer diagnostics integration closeout",
      "work through runtime propagation"};
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
      !contains(ledger_md, required_docs[index]) ||
      !contains(ledger_json, required_docs[index])) {
      return 70 + static_cast<int>(index);
    }
  }

  return 0;
}
