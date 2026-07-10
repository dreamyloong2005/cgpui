#include "cgpui/renderer/renderer_frame_timing_diagnostics.hpp"

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

int test_counts_every_frame_stage() {
  cgpui::RendererFrameTimings timings;
  cgpui::renderer_add_frame_timing(
      timings, cgpui::RendererFrameTimingStage::pacing_wait, 2);
  cgpui::renderer_add_frame_timing(
      timings, cgpui::RendererFrameTimingStage::resource_preparation, 3);
  cgpui::renderer_add_frame_timing(
      timings, cgpui::RendererFrameTimingStage::image_acquisition, 5);
  cgpui::renderer_add_frame_timing(
      timings, cgpui::RendererFrameTimingStage::command_recording, 7);
  cgpui::renderer_add_frame_timing(
      timings, cgpui::RendererFrameTimingStage::queue_submission, 11);
  cgpui::renderer_add_frame_timing(
      timings, cgpui::RendererFrameTimingStage::presentation, 13);
  return timings.pacing_wait_nanoseconds == 2 &&
                 timings.resource_preparation_nanoseconds == 3 &&
                 timings.image_acquisition_nanoseconds == 5 &&
                 timings.command_recording_nanoseconds == 7 &&
                 timings.queue_submission_nanoseconds == 11 &&
                 timings.presentation_nanoseconds == 13 &&
                 timings.total_nanoseconds == 41 && !timings.saturated
             ? 0
             : 10;
}

int test_zero_timing_add_is_a_noop() {
  cgpui::RendererFrameTimings timings;
  cgpui::renderer_add_frame_timing(
      timings, cgpui::RendererFrameTimingStage::presentation, 0);
  return timings.presentation_nanoseconds == 0 &&
                 timings.total_nanoseconds == 0 && !timings.saturated
             ? 0
             : 20;
}

int test_frame_timing_saturates() {
  cgpui::RendererFrameTimings timings;
  cgpui::renderer_add_frame_timing(
      timings,
      cgpui::RendererFrameTimingStage::command_recording,
      std::numeric_limits<std::uint64_t>::max());
  cgpui::renderer_add_frame_timing(
      timings, cgpui::RendererFrameTimingStage::command_recording, 1);
  return timings.command_recording_nanoseconds ==
                     std::numeric_limits<std::uint64_t>::max() &&
                 timings.total_nanoseconds ==
                     std::numeric_limits<std::uint64_t>::max() &&
                 timings.saturated
             ? 0
             : 30;
}

int test_frame_budget_comparison() {
  const cgpui::RendererFrameTimingDiagnostics within =
      cgpui::compare_renderer_frame_timing(
          20, {.total_nanoseconds = 12});
  if (!within.within_budget() || within.remaining_budget_nanoseconds != 8 ||
      within.over_budget_nanoseconds != 0) {
    return 40;
  }
  const cgpui::RendererFrameTimingDiagnostics over =
      cgpui::compare_renderer_frame_timing(
          20, {.total_nanoseconds = 27});
  if (over.within_budget() || over.remaining_budget_nanoseconds != 0 ||
      over.over_budget_nanoseconds != 7) {
    return 41;
  }
  const cgpui::RendererFrameTimingDiagnostics exact =
      cgpui::compare_renderer_frame_timing(
          20, {.total_nanoseconds = 20});
  return exact.within_budget() && exact.remaining_budget_nanoseconds == 0 &&
                 exact.over_budget_nanoseconds == 0
             ? 0
             : 42;
}

int test_step_519_structure_and_documentation() {
  const std::string header = read_source(
      "include/cgpui/renderer/renderer_frame_timing_diagnostics.hpp");
  const std::string source = read_source(
      "src/renderer/renderer_frame_timing_diagnostics.cpp");
  const std::string aggregate =
      read_source("include/cgpui/renderer/renderer_reports.hpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (!contains(header, "struct RendererFrameTimingDiagnostics") ||
      !contains(header, "renderer_add_frame_timing(") ||
      !contains(source, "compare_renderer_frame_timing(") ||
      !contains(aggregate, "renderer_frame_timing_diagnostics.hpp") ||
      !contains(structure, "renderer_frame_timing_diagnostics.cpp")) {
    return 50;
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
      "Phase E Step 519",
      "RendererFrameTimingDiagnostics",
      "explicit CPU frame-stage nanoseconds",
      "saturation-safe timing accumulation",
      "Step 520 live Vulkan diagnostic snapshots",
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
  if (const int result = test_counts_every_frame_stage(); result != 0) {
    return result;
  }
  if (const int result = test_zero_timing_add_is_a_noop(); result != 0) {
    return result;
  }
  if (const int result = test_frame_timing_saturates(); result != 0) {
    return result;
  }
  if (const int result = test_frame_budget_comparison(); result != 0) {
    return result;
  }
  return test_step_519_structure_and_documentation();
}
