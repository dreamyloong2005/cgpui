#include "cgpui/renderer/renderer_frame_diagnostics.hpp"

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

int test_exact_work_matches() {
  const cgpui::RendererFrameDiagnostics diagnostics =
      cgpui::compare_renderer_frame_work(
          {.batch_count = 4, .command_count = 12},
          {.batch_count = 4, .command_count = 12});
  return diagnostics.exact_match() &&
                 diagnostics.planned_work.batch_count == 4 &&
                 diagnostics.submitted_work.command_count == 12 &&
                 diagnostics.pending_batch_count == 0 &&
                 diagnostics.unexpected_batch_count == 0 &&
                 diagnostics.pending_command_count == 0 &&
                 diagnostics.unexpected_command_count == 0
             ? 0
             : 10;
}

int test_under_submission_reports_pending_work() {
  const cgpui::RendererFrameDiagnostics diagnostics =
      cgpui::compare_renderer_frame_work(
          {.batch_count = 5, .command_count = 9},
          {.batch_count = 3, .command_count = 4});
  return !diagnostics.exact_match() &&
                 diagnostics.pending_batch_count == 2 &&
                 diagnostics.pending_command_count == 5 &&
                 diagnostics.unexpected_batch_count == 0 &&
                 diagnostics.unexpected_command_count == 0
             ? 0
             : 20;
}

int test_over_submission_reports_unexpected_work() {
  const cgpui::RendererFrameDiagnostics diagnostics =
      cgpui::compare_renderer_frame_work(
          {.batch_count = 2, .command_count = 7},
          {.batch_count = 6, .command_count = 11});
  return !diagnostics.exact_match() &&
                 diagnostics.pending_batch_count == 0 &&
                 diagnostics.pending_command_count == 0 &&
                 diagnostics.unexpected_batch_count == 4 &&
                 diagnostics.unexpected_command_count == 4
             ? 0
             : 30;
}

int test_empty_work_matches() {
  const cgpui::RendererFrameDiagnostics diagnostics =
      cgpui::compare_renderer_frame_work({}, {});
  return diagnostics.exact_match() ? 0 : 40;
}

int test_structure() {
  const std::string header = read_source(
      "include/cgpui/renderer/renderer_frame_diagnostics.hpp");
  const std::string source =
      read_source("src/renderer/renderer_frame_diagnostics.cpp");
  const std::string aggregate =
      read_source("include/cgpui/renderer/renderer_reports.hpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (!contains(header, "struct RendererFrameWork") ||
      !contains(header, "struct RendererFrameDiagnostics") ||
      !contains(source, "compare_renderer_frame_work(") ||
      !contains(aggregate, "renderer_frame_diagnostics.hpp") ||
      !contains(structure, "renderer_frame_diagnostics.cpp")) {
    return 50;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_exact_work_matches(); result != 0) {
    return result;
  }
  if (const int result = test_under_submission_reports_pending_work();
      result != 0) {
    return result;
  }
  if (const int result = test_over_submission_reports_unexpected_work();
      result != 0) {
    return result;
  }
  if (const int result = test_empty_work_matches(); result != 0) {
    return result;
  }
  return test_structure();
}
