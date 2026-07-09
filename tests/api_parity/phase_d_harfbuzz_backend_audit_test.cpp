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
  const std::string harfbuzz_source =
      read_source("src/ui/text_shaping_harfbuzz.cpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string xmake = read_source("xmake.lua");

  if (harfbuzz_source.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty() ||
      xmake.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_d_harfbuzz_backend_audit_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_d_harfbuzz_backend_audit_test.cpp")) {
    return 2;
  }

  constexpr std::array required_source_fragments{
      "CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND",
      "#include <hb.h>",
      "#include <hb-ot.h>",
      "hb_buffer_add_utf8",
      "hb_buffer_set_direction",
      "hb_buffer_set_script",
      "hb_language_from_string",
      "hb_blob_create",
      "hb_face_create",
      "hb_ot_font_set_funcs",
      "hb_shape",
      "TextShapingFallbackReason::shaping_failed",
      "shape_text_with_deterministic_fallback",
      "harfbuzz_file_backed_face_path",
      "shape_text_with_harfbuzz",
  };
  for (std::size_t index = 0; index < required_source_fragments.size();
       ++index) {
    if (!contains(harfbuzz_source, required_source_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  if (contains(harfbuzz_source, "#error")) {
    return 40;
  }

  constexpr std::array required_docs{
      "Phase D guarded HarfBuzz backend",
      "phase_d_harfbuzz_backend_audit_test.cpp",
      "guarded HarfBuzz backend now shapes through hb_shape",
      "file-backed font faces when available and deterministic fallback on shaping failure",
      "DirectWrite font-file extraction remains later work",
  };
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
        !contains(ledger_md, required_docs[index]) ||
        !contains(ledger_json, required_docs[index]) ||
        !contains(task_plan, required_docs[index]) ||
        !contains(findings, required_docs[index])) {
      return 50 + static_cast<int>(index);
    }
  }

  return 0;
}
