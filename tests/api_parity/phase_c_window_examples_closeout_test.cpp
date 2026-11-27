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
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string manifests = read_source("tests/api_parity/mcpp.toml");

  if (roadmap.empty() || ledger_md.empty() || ledger_json.empty() ||
      vocabulary.empty() || manifests.empty()) {
    return 1;
  }

  if (!contains(manifests,
                "\"phase_c_window_examples_closeout_test.cpp\"") ||
      !contains(manifests,
                "\"phase_c_window_examples_closeout_test.cpp\"")) {
    return 2;
  }

  if (!contains(roadmap,
                "tests/api_parity/"
                "phase_c_window_examples_closeout_test.cpp")) {
    return 3;
  }

  constexpr std::array docs_evidence{
      "examples/api_parity/public_window_examples/main.cpp",
      "api_parity_public_window_examples",
      "phase_c_window_examples_public_api_test.cpp",
      "examples/api_parity/public_window_examples_workflow/main.cpp",
      "api_parity_public_window_examples_workflow",
      "phase_c_window_examples_workflow_test.cpp",
      "examples/api_parity/public_window_examples_widget_catalog/main.cpp",
      "api_parity_public_window_examples_widget_catalog",
      "phase_c_window_examples_widget_catalog_test.cpp",
      "examples/api_parity/public_window_examples_interaction_states/main.cpp",
      "api_parity_public_window_examples_interaction_states",
      "phase_c_window_examples_interaction_states_test.cpp",
      "examples/api_parity/public_window_examples_service_matrix/main.cpp",
      "api_parity_public_window_examples_service_matrix",
      "phase_c_window_examples_service_matrix_test.cpp",
      "phase_c_window_examples_closeout_test.cpp"};
  for (std::size_t index = 0; index < docs_evidence.size(); ++index) {
    if (!contains(ledger_md, docs_evidence[index]) ||
        !contains(ledger_json, docs_evidence[index]) ||
        !contains(vocabulary, docs_evidence[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_json,
                "\"window_examples_status\": \"closed\"")) {
    return 70;
  }

  constexpr std::array closeout_exclusions{
      "ClipboardItem",
      "gpui::test",
      "remain explicit types outside the macro contract",
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
