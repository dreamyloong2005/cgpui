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
  const std::string linux_backends =
      read_source("build/xmake/phase_i_linux_backends.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string text_font_header =
      read_source("include/cgpui/ui/text_font.hpp");
  const std::string text_font_source = read_source("src/ui/text_font.cpp");
  const std::string text_shape_header =
      read_source("include/cgpui/ui/text_shape.hpp");
  const std::string text_shape_source = read_source("src/ui/text_shape.cpp");
  const std::string fallback_source =
      read_source("src/ui/text_shaping_fallback.cpp");
  const std::string win32_font =
      read_source("src/platform/win32/win32_font_discovery.cpp");
  const std::string wayland_fontconfig =
      read_source("src/platform/linux/wayland_fontconfig_discovery.cpp");
  const std::string text_model_test =
      read_source("tests/ui/text_model_test.cpp");

  if (xmake.empty() || linux_backends.empty() || roadmap.empty() || ledger.empty() ||
      task_plan.empty() || findings.empty() || text_font_header.empty() ||
      text_font_source.empty() || text_shape_header.empty() ||
      text_shape_source.empty() || fallback_source.empty() ||
      win32_font.empty() || wayland_fontconfig.empty() ||
      text_model_test.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_d_font_fallback_audit_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_d_font_fallback_audit_test.cpp")) {
    return 2;
  }

  const std::string text_row = line_containing(ledger, "| gpui text system |");
  if (text_row.empty() || !contains(text_row, "Phase D Steps 379-") ||
      !contains(text_row, "Step 394 font fallback band audit") ||
      !contains(text_row, "TextGlyphRun::font_fallback_face_index") ||
      !contains(text_row, "TextFontFallbackRun") ||
      !contains(text_row, "TextMissingGlyphDiagnostic") ||
      !contains(text_row, "TextColorGlyphPlan") ||
      !contains(text_row, "emoji presentation selector planning") ||
      !contains(text_row, "has_emoji_presentation_selector") ||
      !contains(text_row, "emoji ZWJ joiner diagnostic suppression") ||
      !contains(text_row, "TextScriptRun") ||
      !contains(text_row, "TextShapeRun::script_runs") ||
      !contains(text_row, "classify_text_shaping_script") ||
      !contains(text_row, "append_script_run_span") ||
      !contains(text_row, "FontDatabase::resolve_chain_for_codepoint") ||
      !contains(text_row, "wayland_fontconfig_discovery.cpp") ||
      !contains(text_row, "system-optional fontconfig package wiring") ||
      !contains(text_row, "guarded HarfBuzz backend now shapes through hb_shape")) {
    return 3;
  }

  if (!contains(roadmap, "Step 394 closes the font discovery/fallback band") ||
      !contains(roadmap, "- [x] Steps 387-394: Add real font discovery") ||
      !contains(roadmap, "system-optional fontconfig package wiring") ||
      !contains(task_plan, "Step 394 closes the font discovery/fallback band") ||
      !contains(task_plan, "system-optional fontconfig package wiring") ||
      !contains(findings, "Step 394 is an audit-only closeout")) {
    return 4;
  }

  if (!contains(text_font_header, "struct FontUnicodeRange") ||
      !contains(text_font_source, "FontDatabase::resolve_chain_for_codepoint(") ||
      !contains(text_shape_header, "font_fallback_faces") ||
      !contains(text_shape_header, "struct TextScriptRun") ||
      !contains(text_shape_header, "std::vector<TextScriptRun> script_runs") ||
      !contains(text_shape_source, "copy_font_fallback_faces(") ||
      !contains(fallback_source, "select_font_fallback_face_index(") ||
      !contains(fallback_source, "append_font_fallback_run_span(") ||
      !contains(fallback_source, "append_missing_glyph_diagnostic(") ||
      !contains(fallback_source, "append_color_glyph_plan(") ||
      !contains(fallback_source,
                "append_emoji_presentation_color_glyph_plan(") ||
      !contains(fallback_source, "mark_emoji_presentation_selector_span(") ||
      !contains(fallback_source, "is_emoji_sequence_joiner(") ||
      !contains(fallback_source,
                "is_emoji_sequence_joiner_between_emoji(") ||
      !contains(fallback_source, "classify_text_shaping_script(") ||
      !contains(fallback_source, "append_script_run_span(") ||
      !contains(fallback_source, "decode_utf8_codepoint(")) {
    return 5;
  }

  if (!contains(win32_font, "DWriteCreateFactory") ||
      !contains(win32_font, "GetSystemFontCollection") ||
      !contains(wayland_fontconfig,
                "CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND") ||
      !contains(wayland_fontconfig, "FcFontList") ||
      !contains(xmake, "add_requires(\"fontconfig\"") ||
      !contains(linux_backends, "has_package(\"fontconfig\")") ||
      !contains(linux_backends, "CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND") ||
      !contains(linux_backends, "add_packages(\"fontconfig\")")) {
    return 6;
  }

  if (!contains(text_model_test,
                "test_font_database_resolves_codepoint_coverage_chain") ||
      !contains(text_model_test,
                "test_shape_text_preserves_font_fallback_chain") ||
      !contains(text_model_test,
                "test_shape_text_records_glyph_fallback_face_indices") ||
      !contains(text_model_test,
                "test_shape_text_splits_contiguous_font_fallback_runs") ||
      !contains(text_model_test,
                "test_shape_text_records_missing_glyph_diagnostics") ||
      !contains(text_model_test,
                "test_shape_text_records_color_glyph_plans") ||
      !contains(text_model_test, "variation_run.color_glyphs.size()") ||
      !contains(text_model_test, "emoji_variation_run.color_glyphs.size()") ||
      !contains(text_model_test,
                "emoji_presentation_selector_byte_length") ||
      !contains(text_model_test, "zwj_run.color_glyphs.size()") ||
      !contains(text_model_test, "test_shape_text_splits_script_runs")) {
    return 7;
  }

  return 0;
}
