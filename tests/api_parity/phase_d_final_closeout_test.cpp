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
  const std::string manifests = read_source("tests/api_parity/mcpp.toml");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string win32_ime =
      read_source("src/platform/win32/win32_window_ime.cpp");
  const std::string win32_ime_placement =
      read_source("src/platform/win32/win32_window_ime_placement.cpp");
  const std::string platform_structure =
      read_source("tests/architecture/platform_source_structure_test.cpp");
  const std::string win32_structure =
      read_source("tests/architecture/win32_window_source_test.cpp");

  if (manifests.empty() ||
      roadmap.empty() ||
      ledger_md.empty() ||
      ledger_json.empty() ||
      win32_ime.empty() ||
      win32_ime_placement.empty() ||
      platform_structure.empty() ||
      win32_structure.empty()) {
    return 1;
  }

  if (!contains(manifests, "\"phase_d_final_closeout_test.cpp\"") ||
      !contains(manifests,
                "\"phase_d_final_closeout_test.cpp\"")) {
    return 2;
  }

  constexpr std::array required_docs{
      "tests/api_parity/phase_d_final_closeout_test.cpp",
      "DirectWrite font-file extraction",
      "color glyph rendering",
      "native ZWJ ligature shaping depth",
      "full Unicode script data",
      "bidirectional",
      "shaping/reordering",
      "paragraph shaping",
      "eviction policy",
      "platform-derived font metrics",
      "Unicode line-break classes",
      "FreeType metrics",
      "richer per-face coverage",
      "preedit styling",
      "production candidate UI policy",
      "runtime rich-text element dispatch",
      "inline image drawing/loading"};
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
      !contains(ledger_md, required_docs[index]) ||
      !contains(ledger_json, required_docs[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  if (!contains(win32_ime, "Win32Window::ime_composition(") ||
      !contains(win32_ime, "ImmGetCompositionStringW") ||
      contains(win32_ime, "ImmSetCandidateWindow") ||
      !contains(win32_ime_placement,
                "Win32Window::apply_ime_text_input_placement()") ||
      !contains(win32_ime_placement, "ImmSetCandidateWindow") ||
      !contains(win32_ime_placement, "ImmSetCompositionWindow") ||
      contains(win32_ime_placement, "GCS_COMPSTR")) {
    return 60;
  }

  if (!contains(platform_structure, "win32_window_ime_placement.cpp") ||
      !contains(win32_structure, "win32_window_ime_placement.cpp") ||
      !contains(platform_structure, "line_count(win32_window_ime_placement)") ||
      !contains(win32_structure, "line_count(win32_window_ime_placement)")) {
    return 70;
  }

  return 0;
}
