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

std::string line_containing(const std::string& text, const char* anchor) {
  const std::size_t anchor_index = text.find(anchor);
  if (anchor_index == std::string::npos) {
    return {};
  }
  const std::size_t line_start = text.rfind('\n', anchor_index);
  const std::size_t line_end = text.find('\n', anchor_index);
  const std::size_t start =
      line_start == std::string::npos ? 0 : line_start + 1U;
  const std::size_t end =
      line_end == std::string::npos ? text.size() : line_end;
  return text.substr(start, end - start);
}

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string measurement_header =
      read_source("include/cgpui/ui/text_measurement.hpp");
  const std::string wrapping_header =
      read_source("include/cgpui/ui/text_wrapping.hpp");
  const std::string paragraph_header =
      read_source("include/cgpui/ui/text_paragraph_layout.hpp");
  const std::string grapheme_source =
      read_source("src/ui/text_measurement_grapheme.cpp");
  const std::string wrapping_source = read_source("src/ui/text_wrapping.cpp");
  const std::string paragraph_source =
      read_source("src/ui/text_paragraph_layout.cpp");
  const std::string structure_test =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string text_model_test =
      read_source("tests/ui/text_model_test.cpp");

  if (xmake.empty() || roadmap.empty() || ledger.empty() ||
      task_plan.empty() || findings.empty() || measurement_header.empty() ||
      wrapping_header.empty() || paragraph_header.empty() ||
      grapheme_source.empty() || wrapping_source.empty() ||
      paragraph_source.empty() || structure_test.empty() ||
      text_model_test.empty()) {
    return 1;
  }

  if (!contains(xmake,
                "target(\"phase_d_text_measurement_wrapping_audit_test\")") ||
      !contains(xmake,
                "tests/api_parity/"
                "phase_d_text_measurement_wrapping_audit_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "- [x] Steps 403-410: Complete text measurement and "
                "wrapping") ||
      !contains(roadmap,
                "Step 410 closes the text measurement/wrapping band through") ||
      contains(roadmap, "- [ ] Steps 403-410")) {
    return 3;
  }

  if (!contains(task_plan,
                "Step 410 closes the text measurement/wrapping band") ||
      !contains(findings, "Step 410 is an audit-only closeout")) {
    return 4;
  }

  const std::string text_row = line_containing(ledger, "| gpui text system |");
  if (text_row.empty() || !contains(text_row, "Phase D Steps 379-410") ||
      !contains(text_row, "Step 410 text measurement/wrapping band audit") ||
      !contains(text_row, "TextGraphemeColumn") ||
      !contains(text_row, "TextWrapBreakKind") ||
      !contains(text_row, "TextBidiRun") ||
      !contains(text_row, "TextLineMetrics") ||
      !contains(text_row, "TextParagraphLayoutCache") ||
      !contains(text_row, "text_measurement_is_crlf_pair") ||
      !contains(text_row, "Unicode line-break classes") ||
      contains(text_row, "CRLF normalization")) {
    return 5;
  }

  constexpr std::array measurement_evidence{
      "struct TextGraphemeColumn",
      "std::vector<TextGraphemeColumn> grapheme_columns",
      "struct TextBidiRun",
      "std::vector<TextBidiRun> bidi_runs",
      "struct TextLineMetrics",
      "TextLineMetrics line_metrics",
  };
  for (std::size_t index = 0; index < measurement_evidence.size(); ++index) {
    if (!contains(measurement_header, measurement_evidence[index])) {
      return 20 + static_cast<int>(index);
    }
  }

  constexpr std::array wrapping_evidence{
      "enum class TextWrapBreakKind",
      "std::size_t column_start",
      "std::size_t bidi_run_start",
      "TextLineMetrics metrics",
      "wrap_text_measurement(",
  };
  for (std::size_t index = 0; index < wrapping_evidence.size(); ++index) {
    if (!contains(wrapping_header, wrapping_evidence[index])) {
      return 40 + static_cast<int>(index);
    }
  }

  constexpr std::array paragraph_evidence{
      "struct TextParagraphLayout",
      "struct TextParagraphLayoutResult",
      "class TextParagraphLayoutCache",
      "layout_text_paragraph(",
  };
  for (std::size_t index = 0; index < paragraph_evidence.size(); ++index) {
    if (!contains(paragraph_header, paragraph_evidence[index])) {
      return 60 + static_cast<int>(index);
    }
  }

  constexpr std::array source_evidence{
      "text_grapheme_column_includes_codepoint(",
      "text_measurement_is_crlf_pair(",
      "text_wrap_line_for_column_range(",
      "text_wrap_line_assign_bidi_runs(",
      "text_wrap_column_is_hard_break(",
      "wrap_text_measurement(measurement, max_width)",
  };
  for (std::size_t index = 0; index < source_evidence.size(); ++index) {
    const bool in_grapheme = contains(grapheme_source, source_evidence[index]);
    const bool in_wrapping = contains(wrapping_source, source_evidence[index]);
    const bool in_paragraph = contains(paragraph_source, source_evidence[index]);
    if (!in_grapheme && !in_wrapping && !in_paragraph) {
      return 80 + static_cast<int>(index);
    }
    if (!contains(structure_test, source_evidence[index])) {
      return 90 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      "test_text_measurement_records_grapheme_columns",
      "test_text_soft_wrap_respects_grapheme_columns",
      "test_text_hard_wrap_records_newline_lines",
      "test_text_measurement_records_bidi_runs",
      "test_text_measurement_records_line_metrics",
      "test_text_paragraph_layout_cache_reuses_wrapped_layouts",
      "test_text_hard_wrap_normalizes_crlf_breaks",
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    if (!contains(text_model_test, behavior_evidence[index])) {
      return 110 + static_cast<int>(index);
    }
  }

  return 0;
}
