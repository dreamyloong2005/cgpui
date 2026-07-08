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
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string backend_header =
      read_source("include/cgpui/ui/text_shaping_backend.hpp");
  const std::string shape_header = read_source("include/cgpui/ui/text_shape.hpp");
  const std::string backend_source =
      read_source("src/ui/text_shaping_backend.cpp");
  const std::string dispatch_source =
      read_source("src/ui/text_shaping_dispatch.cpp");
  const std::string harfbuzz_source =
      read_source("src/ui/text_shaping_harfbuzz.cpp");
  const std::string fallback_source =
      read_source("src/ui/text_shaping_fallback.cpp");
  const std::string wayland_fontconfig_source =
      read_source("src/platform/linux/wayland_fontconfig_discovery.cpp");
  const std::string text_model_test =
      read_source("tests/ui/text_model_test.cpp");

  if (xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      task_plan.empty() || findings.empty() || backend_header.empty() ||
      shape_header.empty() || backend_source.empty() ||
      dispatch_source.empty() || harfbuzz_source.empty() ||
      fallback_source.empty() || wayland_fontconfig_source.empty() ||
      text_model_test.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_d_text_shaping_audit_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_d_text_shaping_audit_test.cpp") ||
      !contains(xmake, "target(\"wayland_font_discovery_test\")") ||
      !contains(xmake, "tests/platform/wayland_font_discovery_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "Step 386 adds this text-shaping readiness audit") ||
      !contains(task_plan, "Step 386 adds a text-shaping readiness audit") ||
      !contains(findings,
                "Step 386 is a readiness audit rather than a false "
                "HarfBuzz completion") ||
      contains(roadmap,
               "- [x] Steps 379-386: Replace fallback-only shaping with "
               "HarfBuzz-backed")) {
    return 3;
  }

  const std::string text_row =
      line_containing(ledger_md, "| gpui text system |");
  if (text_row.empty() || !contains(text_row, "Phase D Steps 379-") ||
      !contains(text_row, "text-shaping readiness audit") ||
      !contains(text_row, "PlatformFontDiscoveryResult") ||
      !contains(text_row, "wayland_fontconfig_discovery.cpp") ||
      !contains(text_row, "FontUnicodeRange") ||
      !contains(text_row, "FontDatabase::resolve_chain_for_codepoint") ||
      !contains(text_row, "TextShapeRun::font_fallback_faces") ||
      !contains(text_row, "TextGlyphRun::font_fallback_face_index") ||
      !contains(text_row, "TextFontFallbackRun") ||
      !contains(text_row, "TextShapeRun::font_runs") ||
      !contains(text_row, "TextMissingGlyphDiagnostic") ||
      !contains(text_row, "TextShapeRun::missing_glyphs") ||
      !contains(text_row, "TextColorGlyphPlan") ||
      !contains(text_row, "TextShapeRun::color_glyphs") ||
      !contains(text_row, "emoji presentation selector planning") ||
      !contains(text_row, "U+FE0F") ||
      !contains(text_row, "has_emoji_presentation_selector") ||
      !contains(text_row, "emoji_presentation_selector_byte_offset") ||
      !contains(text_row, "emoji ZWJ joiner diagnostic suppression") ||
      !contains(text_row, "U+200D") ||
      !contains(text_row, "production HarfBuzz shaping") ||
      !contains(text_row, "native ZWJ ligature shaping") ||
      !contains(text_row, "native Linux fontconfig/FreeType font enumeration")) {
    return 4;
  }

  if (!contains(backend_header, "TextShapingBackendCapabilities capabilities") ||
      !contains(shape_header, "TextShapingBackendCapabilities backend_capabilities") ||
      !contains(shape_header, "TextShapingBackendSelection backend_selection()") ||
      !contains(shape_header, "bool used_fallback() const") ||
      !contains(shape_header, "struct TextFontFallbackRun") ||
      !contains(shape_header, "std::vector<TextFontFallbackRun> font_runs") ||
      !contains(shape_header, "struct TextMissingGlyphDiagnostic") ||
      !contains(shape_header,
                "std::vector<TextMissingGlyphDiagnostic> missing_glyphs") ||
      !contains(shape_header, "enum class TextColorGlyphFormat") ||
      !contains(shape_header, "struct TextColorGlyphPlan") ||
      !contains(shape_header, "std::vector<TextColorGlyphPlan> color_glyphs") ||
      !contains(shape_header, "bool has_emoji_presentation_selector") ||
      !contains(shape_header, "emoji_presentation_selector_byte_offset") ||
      !contains(shape_header, "emoji_presentation_selector_byte_length") ||
      !contains(backend_source, "CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND")) {
    return 5;
  }

  if (!contains(dispatch_source, "shape_text_with_harfbuzz") ||
      !contains(dispatch_source, "TextShapingBackend::harfbuzz") ||
      !contains(harfbuzz_source,
                "CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND requires the real "
                "HarfBuzz shaping implementation") ||
      !contains(harfbuzz_source,
                "TextShapingFallbackReason::backend_unavailable") ||
      !contains(fallback_source, ".backend_capabilities")) {
    return 6;
  }
  if (!contains(fallback_source, "is_emoji_presentation_selector(") ||
      !contains(fallback_source, "codepoint_accepts_emoji_presentation(") ||
      !contains(fallback_source,
                "append_emoji_presentation_color_glyph_plan(") ||
      !contains(fallback_source, "mark_emoji_presentation_selector_span(")) {
    return 10;
  }
  if (!contains(fallback_source, "is_emoji_sequence_joiner(") ||
      !contains(fallback_source, "next_utf8_codepoint_after(") ||
      !contains(fallback_source,
                "is_emoji_sequence_joiner_between_emoji(")) {
    return 11;
  }

  if (!contains(wayland_fontconfig_source,
                "CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND") ||
      !contains(wayland_fontconfig_source, "FcFontList") ||
      !contains(wayland_fontconfig_source,
                "PlatformFontDiscoveryStatus::native_available") ||
      !contains(findings,
                "native Linux font discovery cannot honestly be marked "
                "complete")) {
    return 8;
  }

  if (!contains(text_model_test, "default_run.backend_selection()") ||
      !contains(text_model_test, "default_run.used_fallback()") ||
      !contains(text_model_test, "fallback_run.used_fallback()") ||
      !contains(text_model_test, "selection.capabilities.harfbuzz_available") ||
      !contains(text_model_test,
                "test_shape_text_splits_contiguous_font_fallback_runs") ||
      !contains(text_model_test,
                "test_shape_text_records_missing_glyph_diagnostics") ||
      !contains(text_model_test,
                "test_shape_text_records_color_glyph_plans") ||
      !contains(text_model_test, "variation_run.color_glyphs.size()") ||
      !contains(text_model_test, "emoji_variation_run.color_glyphs.size()") ||
      !contains(text_model_test,
                "emoji_presentation_selector_byte_offset") ||
      !contains(text_model_test, "zwj_run.missing_glyphs.empty()")) {
    return 9;
  }

  return 0;
}
