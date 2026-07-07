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

  if (!contains(xmake, "target(\"phase_c_uniform_list_audit_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_c_uniform_list_audit_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "- [x] Steps 349-354: Implement uniform list parity") ||
      !contains(roadmap,
                "Step 354 closes the uniform-list band through") ||
      !contains(roadmap,
                "tests/api_parity/phase_c_uniform_list_audit_test.cpp") ||
      !contains(roadmap,
                "Step 355 starts the window/examples widget band")) {
    return 3;
  }
  if (contains(roadmap, "Step 354 should close")) {
    return 4;
  }

  constexpr std::array roadmap_evidence{
      "Step 349 starts uniform list parity",
      "Step 350 adds keyed scroll anchoring",
      "Step 351 adds keyed item measurement caching",
      "Step 352 adds `UniformListRecyclingWindow`",
      "Step 353 adds `UniformListSelectionSource`",
  };
  for (std::size_t index = 0; index < roadmap_evidence.size(); ++index) {
    if (!contains(roadmap, roadmap_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array ledger_evidence{
      "Phase C Step 349 uniform list parity",
      "UniformListVisibleRange",
      "UniformListItemIdentity",
      "Phase C Step 350 scroll anchoring",
      "UniformListScrollAnchor",
      "Phase C Step 351 item measurement cache",
      "UniformListItemMeasurementCache",
      "Phase C Step 352 large-list recycling",
      "UniformListRecyclingWindow",
      "Phase C Step 353 keyboard/pointer selection",
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
      "Phase C Step 354 uniform list closeout",
      "phase_c_uniform_list_audit_test.cpp",
  };
  for (std::size_t index = 0; index < ledger_evidence.size(); ++index) {
    if (!contains(ledger_md, ledger_evidence[index]) ||
        !contains(ledger_json, ledger_evidence[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_md, "| gpui uniform_list |") ||
      !contains(ledger_md, "| Adapted |") ||
      !contains(ledger_json, "\"upstream_gpui\": \"gpui uniform_list\"") ||
      !contains(ledger_json, "\"status\": \"adapted\"") ||
      !contains(ledger_json,
                "\"next_step\": \"Phase C uniform-list band closed\"") ||
      !contains(ledger_json,
                "\"next_step\": \"Phase C Step 355 window/examples widgets\"")) {
    return 60;
  }
  if (contains(ledger_md, "uniform-list band closeout remains") ||
      contains(ledger_json, "uniform-list band closeout remains") ||
      contains(ledger_json,
               "\"next_step\": \"Phase C Step 354 uniform list closeout\"")) {
    return 61;
  }

  if (!contains(vocabulary, "Phase C Step 354 closes the uniform-list band") ||
      !contains(vocabulary,
                "tests/api_parity/phase_c_uniform_list_audit_test.cpp")) {
    return 70;
  }

  constexpr std::array closeout_exclusions{
      "ClipboardItem",
      "gpui::test",
      "action macro payloads",
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
