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
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string xmake = read_source("xmake.lua");

  if (roadmap.empty() || ledger_md.empty() || ledger_json.empty() ||
      vocabulary.empty() || xmake.empty()) {
    return 1;
  }

  if (!contains(xmake,
                "target(\"phase_c_window_examples_closeout_test\")") ||
      !contains(xmake,
                "tests/api_parity/"
                "phase_c_window_examples_closeout_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "- [x] Steps 355-360: Implement window/examples widgets") ||
      !contains(roadmap,
                "Step 360 closes the window/examples widget band through") ||
      !contains(roadmap,
                "tests/api_parity/"
                "phase_c_window_examples_closeout_test.cpp") ||
      !contains(roadmap,
                "Step 361 starts SVG/image element front-end APIs")) {
    return 3;
  }
  if (contains(roadmap, "- [ ] Steps 355-360") ||
      contains(roadmap, "Step 360 should close")) {
    return 4;
  }

  constexpr std::array roadmap_evidence{
      "Phase C Step 355 window/examples widgets starts this band",
      "Phase C Step 356 window/examples workflow deepens the window/examples widget band",
      "Phase C Step 357 window/examples widget catalog adds",
      "Phase C Step 358 window/examples interaction states adds",
      "Phase C Step 359 window/examples service matrix adds",
  };
  for (std::size_t index = 0; index < roadmap_evidence.size(); ++index) {
    if (!contains(roadmap, roadmap_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array docs_evidence{
      "Phase C Step 355 window/examples widgets",
      "examples/api_parity/public_window_examples/main.cpp",
      "api_parity_public_window_examples",
      "phase_c_window_examples_public_api_test.cpp",
      "Phase C Step 356 window/examples workflow",
      "examples/api_parity/public_window_examples_workflow/main.cpp",
      "api_parity_public_window_examples_workflow",
      "phase_c_window_examples_workflow_test.cpp",
      "Phase C Step 357 window/examples widget catalog",
      "examples/api_parity/public_window_examples_widget_catalog/main.cpp",
      "api_parity_public_window_examples_widget_catalog",
      "phase_c_window_examples_widget_catalog_test.cpp",
      "Phase C Step 358 window/examples interaction states",
      "examples/api_parity/public_window_examples_interaction_states/main.cpp",
      "api_parity_public_window_examples_interaction_states",
      "phase_c_window_examples_interaction_states_test.cpp",
      "Phase C Step 359 window/examples service matrix",
      "examples/api_parity/public_window_examples_service_matrix/main.cpp",
      "api_parity_public_window_examples_service_matrix",
      "phase_c_window_examples_service_matrix_test.cpp",
      "Phase C Step 360 window/examples closeout",
      "phase_c_window_examples_closeout_test.cpp",
      "Phase C Step 361 SVG/image element front-end APIs",
  };
  for (std::size_t index = 0; index < docs_evidence.size(); ++index) {
    if (!contains(ledger_md, docs_evidence[index]) ||
        !contains(ledger_json, docs_evidence[index]) ||
        !contains(vocabulary, docs_evidence[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_json,
                "\"window_examples_status\": \"closed\"") ||
      !contains(ledger_json,
                "\"window_examples_next\": \"Phase C Step 360 closes the "
                "window/examples widget band. Phase C Step 361 SVG/image "
                "element front-end APIs")) {
    return 70;
  }
  if (contains(ledger_json,
               "\"next_step\": \"Phase C Step 360 window/examples "
               "closeout\"") ||
      contains(ledger_md, "Step 360 should close") ||
      contains(ledger_json, "Step 360 should close") ||
      contains(vocabulary, "Step 360 should close")) {
    return 71;
  }

  constexpr std::array closeout_exclusions{
      "ClipboardItem",
      "gpui::test",
      "action macro payloads",
      "task priorities",
      "structured task groups",
      "private runtime headers",
      "WindowRuntime",
  };
  for (std::size_t index = 0; index < closeout_exclusions.size(); ++index) {
    if (!contains(roadmap, closeout_exclusions[index]) ||
        !contains(vocabulary, closeout_exclusions[index])) {
      return 90 + static_cast<int>(index);
    }
  }

  return 0;
}
