#include <array>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  std::ifstream source(path);
  if (!source) {
    source.open((std::string("../../../../") + path).c_str());
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
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string xmake = read_source("xmake.lua");

  if (roadmap.empty() || vocabulary.empty() || ledger_md.empty() ||
      ledger_json.empty() || xmake.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"public_phase_b_completion_audit_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/public_phase_b_completion_audit_test.cpp")) {
    return 2;
  }

  if (!contains(
          roadmap,
          "- [x] Steps 313-318: Run full Windows/WSL verification and freeze") ||
      !contains(roadmap, "Step 319") ||
      !contains(roadmap, "landed the child-list") ||
      !contains(roadmap, "Phase B public authoring boundary") ||
      !contains(roadmap, "Keep the Phase B closeout") ||
      !contains(roadmap, "exclusions out")) {
    return 3;
  }

  if (!contains(vocabulary, "Phase B Steps 313-318") ||
      !contains(vocabulary, "final Phase B freeze") ||
      !contains(vocabulary, "Phase C starts from this frozen boundary") ||
      contains(vocabulary, "future Phase B or later work")) {
    return 4;
  }

  if (!contains(ledger_md,
                "tests/api_parity/public_phase_b_completion_audit_test.cpp") ||
      !contains(ledger_json,
                "tests/api_parity/public_phase_b_completion_audit_test.cpp") ||
      !contains(ledger_md, "final Phase B public vocabulary completion audit") ||
      !contains(ledger_json,
                "final Phase B public vocabulary completion audit")) {
    return 5;
  }

  constexpr std::array public_example_paths{
      "examples/api_parity/public_api_compatibility/main.cpp",
      "examples/api_parity/public_authoring_workflow/main.cpp",
      "examples/api_parity/public_context_capabilities/main.cpp",
      "examples/api_parity/public_async_test_workflow/main.cpp",
      "examples/api_parity/public_phase_b_surface_closure/main.cpp",
  };
  constexpr std::array public_example_targets{
      "target(\"api_parity_public_api_compatibility\")",
      "target(\"api_parity_public_authoring_workflow\")",
      "target(\"api_parity_public_context_capabilities\")",
      "target(\"api_parity_public_async_test_workflow\")",
      "target(\"api_parity_public_phase_b_surface_closure\")",
  };
  for (std::size_t index = 0; index < public_example_paths.size(); ++index) {
    if (!contains(xmake, public_example_paths[index]) ||
        !contains(xmake, public_example_targets[index])) {
      return 10 + static_cast<int>(index);
    }

    const std::string example = read_source(public_example_paths[index]);
    if (example.empty()) {
      return 20 + static_cast<int>(index);
    }
    if (!contains(example, "#include \"cgpui/prelude.hpp\"")) {
      return 30 + static_cast<int>(index);
    }

    constexpr std::array forbidden_public_example_terms{
        "WindowRuntime",
        "#include \"cgpui/ui/",
        "#include \"cgpui/platform/",
        "#include \"cgpui/renderer/",
        "#include \"src/",
        "#include \"../",
        "ClipboardItem",
        "gpui::test",
        "TaskPriority",
        "StructuredTaskGroup",
        ".runtime",
        "runtime.",
        "window_runtime",
    };
    for (std::size_t term = 0; term < forbidden_public_example_terms.size();
         ++term) {
      if (contains(example, forbidden_public_example_terms[term])) {
        return 40 + static_cast<int>(index * 16 + term);
      }
    }
  }

  constexpr std::array public_test_paths{
      "tests/api_parity/public_api_compatibility_examples_test.cpp",
      "tests/api_parity/public_api_example_expansion_test.cpp",
      "tests/api_parity/public_context_capability_example_test.cpp",
      "tests/api_parity/public_async_test_workflow_example_test.cpp",
      "tests/api_parity/public_phase_b_surface_closure_example_test.cpp",
      "tests/api_parity/public_authoring_vocabulary_freeze_test.cpp",
      "tests/api_parity/public_phase_b_completion_audit_test.cpp",
  };
  constexpr std::array public_test_targets{
      "target(\"public_api_compatibility_examples_test\")",
      "target(\"public_api_example_expansion_test\")",
      "target(\"public_context_capability_example_test\")",
      "target(\"public_async_test_workflow_example_test\")",
      "target(\"public_phase_b_surface_closure_example_test\")",
      "target(\"public_authoring_vocabulary_freeze_test\")",
      "target(\"public_phase_b_completion_audit_test\")",
  };
  for (std::size_t index = 0; index < public_test_paths.size(); ++index) {
    if (!contains(xmake, public_test_paths[index]) ||
        !contains(xmake, public_test_targets[index])) {
      return 150 + static_cast<int>(index);
    }
  }

  constexpr std::array deferred_exclusions{
      "ClipboardItem",
      "gpui::test",
      "remain explicit types outside the macro contract",
      "task priorities",
      "structured task groups",
  };
  for (std::size_t index = 0; index < deferred_exclusions.size(); ++index) {
    if (!contains(roadmap, deferred_exclusions[index]) ||
        !contains(vocabulary, deferred_exclusions[index])) {
      return 180 + static_cast<int>(index);
    }
  }

  return 0;
}
