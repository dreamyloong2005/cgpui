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

  if (!contains(manifests, "\"phase_c_uniform_list_audit_test.cpp\"") ||
      !contains(manifests,
                "\"phase_c_uniform_list_audit_test.cpp\"")) {
    return 2;
  }

  if (!contains(roadmap,
                "tests/api_parity/phase_c_uniform_list_audit_test.cpp")) {
    return 3;
  }

  constexpr std::array ledger_evidence{
      "UniformListVisibleRange",
      "UniformListItemIdentity",
      "UniformListScrollAnchor",
      "UniformListItemMeasurementCache",
      "UniformListRecyclingWindow",
      "UniformListSelectionState",
      "src/ui/uniform_list.cpp",
      "src/ui/uniform_list_measurement.cpp",
      "src/ui/uniform_list_recycling.cpp",
      "src/ui/uniform_list_selection.cpp",
      "src/ui/element_scroll_layout.cpp",
      "src/ui/element_scroll_events.cpp",
      "tests/ui/scroll_test.cpp",
      "tests/ui/element_test.cpp",
      "tests/architecture/ui_source_structure_test.cpp",
      "phase_c_uniform_list_audit_test.cpp"};
  for (std::size_t index = 0; index < ledger_evidence.size(); ++index) {
    if (!contains(ledger_md, ledger_evidence[index]) ||
        !contains(ledger_json, ledger_evidence[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_md, "| gpui uniform_list |") ||
      !contains(ledger_md, "| Adapted |") ||
      !contains(ledger_json, "\"upstream_gpui\": \"gpui uniform_list\"") ||
      !contains(ledger_json, "\"status\": \"adapted\"")) {
    return 60;
  }
  if (!contains(vocabulary,
                "tests/api_parity/phase_c_uniform_list_audit_test.cpp")) {
    return 70;
  }

  constexpr std::array closeout_exclusions{
      "ClipboardItem",
      "gpui::test",
      "remain explicit types outside the macro contract",
      "task priorities",
      "structured task groups",
  };
  for (std::size_t index = 0; index < closeout_exclusions.size(); ++index) {
    if (!contains(roadmap, closeout_exclusions[index]) ||
        !contains(vocabulary, closeout_exclusions[index])) {
      return 80 + static_cast<int>(index);
    }
  }

  return 0;
}
