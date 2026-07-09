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
      line_start == std::string::npos ? 0 : line_start + 1;
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
  const std::string text_shape_header =
      read_source("include/cgpui/ui/text_shape.hpp");
  const std::string fallback_source =
      read_source("src/ui/text_shaping_fallback.cpp");
  const std::string structure_test =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string text_model_test =
      read_source("tests/ui/text_model_test.cpp");
  const std::string text_shaping_audit =
      read_source("tests/api_parity/phase_d_text_shaping_audit_test.cpp");
  const std::string font_fallback_audit =
      read_source("tests/api_parity/phase_d_font_fallback_audit_test.cpp");

  if (xmake.empty() || roadmap.empty() || ledger.empty() ||
      task_plan.empty() || findings.empty() || text_shape_header.empty() ||
      fallback_source.empty() || structure_test.empty() ||
      text_model_test.empty() || text_shaping_audit.empty() ||
      font_fallback_audit.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_d_fallback_splitting_audit_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_d_fallback_splitting_audit_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "- [x] Steps 395-402: Add per-script and per-codepoint "
                "fallback splitting") ||
      !contains(roadmap,
                "Step 402 closes the fallback metadata band through") ||
      !contains(roadmap,
                "tests/api_parity/"
                "phase_d_fallback_splitting_audit_test.cpp") ||
      !contains(roadmap, "Step 403 starts text measurement and wrapping")) {
    return 3;
  }
  if (contains(roadmap, "- [ ] Steps 395-402") ||
      contains(roadmap, "Step 402 should close")) {
    return 4;
  }

  const std::string text_row = line_containing(ledger, "| gpui text system |");
  if (text_row.empty() || !contains(text_row, "Phase D Steps 379-402") ||
      !contains(text_row, "Step 402 fallback metadata band audit") ||
      !contains(text_row, "TextFontFallbackRun") ||
      !contains(text_row, "TextShapeRun::font_runs") ||
      !contains(text_row, "TextMissingGlyphDiagnostic") ||
      !contains(text_row, "TextShapeRun::missing_glyphs") ||
      !contains(text_row, "TextColorGlyphPlan") ||
      !contains(text_row, "TextShapeRun::color_glyphs") ||
      !contains(text_row, "U+FE0F") || !contains(text_row, "U+200D") ||
      !contains(text_row, "TextScriptRun") ||
      !contains(text_row, "TextShapeRun::script_runs") ||
      !contains(text_row, "full Unicode script data") ||
      !contains(text_row, "bidirectional shaping") ||
      !contains(text_row, "production HarfBuzz shaping")) {
    return 5;
  }

  if (!contains(task_plan, "Step 402 closes the fallback metadata band") ||
      !contains(findings, "Step 402 is an audit-only closeout")) {
    return 6;
  }

  constexpr std::array header_evidence{
      "struct TextFontFallbackRun",
      "std::vector<TextFontFallbackRun> font_runs",
      "struct TextMissingGlyphDiagnostic",
      "std::vector<TextMissingGlyphDiagnostic> missing_glyphs",
      "struct TextColorGlyphPlan",
      "std::vector<TextColorGlyphPlan> color_glyphs",
      "struct TextScriptRun",
      "std::vector<TextScriptRun> script_runs",
  };
  for (std::size_t index = 0; index < header_evidence.size(); ++index) {
    if (!contains(text_shape_header, header_evidence[index])) {
      return 20 + static_cast<int>(index);
    }
  }

  constexpr std::array fallback_evidence{
      "append_font_fallback_run_span(",
      "append_missing_glyph_diagnostic(",
      "append_color_glyph_plan(",
      "append_emoji_presentation_color_glyph_plan(",
      "mark_emoji_presentation_selector_span(",
      "is_emoji_sequence_joiner_between_emoji(",
      "classify_text_shaping_script(",
      "append_script_run_span(",
  };
  for (std::size_t index = 0; index < fallback_evidence.size(); ++index) {
    if (!contains(fallback_source, fallback_evidence[index]) ||
        !contains(structure_test, fallback_evidence[index])) {
      return 40 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      "test_shape_text_splits_contiguous_font_fallback_runs",
      "test_shape_text_records_missing_glyph_diagnostics",
      "test_shape_text_records_color_glyph_plans",
      "test_shape_text_splits_script_runs",
      "emoji_presentation_selector_byte_offset",
      "zwj_run.missing_glyphs.empty()",
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    if (!contains(text_model_test, behavior_evidence[index])) {
      return 60 + static_cast<int>(index);
    }
  }

  if (!contains(text_shaping_audit, "TextScriptRun") ||
      !contains(font_fallback_audit, "TextScriptRun")) {
    return 80;
  }

  return 0;
}
