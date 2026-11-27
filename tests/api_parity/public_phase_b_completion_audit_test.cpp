#include <array>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  std::ifstream source(std::string{CGPUI_SOURCE_ROOT} + "/" + path);
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
  const std::string manifests = read_source("tests/api_parity/mcpp.toml") +
      read_source("examples/api_parity/mcpp.toml");

  if (roadmap.empty() || vocabulary.empty() || ledger_md.empty() ||
      ledger_json.empty() || manifests.empty()) {
    return 1;
  }

  if (!contains(manifests, "\"public_phase_b_completion_audit_test.cpp\"") ||
      !contains(
          manifests,
          "\"public_phase_b_completion_audit_test.cpp\"")) {
    return 2;
  }

  if (!contains(ledger_md,
                "tests/api_parity/public_phase_b_completion_audit_test.cpp") ||
      !contains(ledger_json,
                "tests/api_parity/public_phase_b_completion_audit_test.cpp")) {
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
      "[targets.api_parity_public_api_compatibility]",
      "[targets.api_parity_public_authoring_workflow]",
      "[targets.api_parity_public_context_capabilities]",
      "[targets.api_parity_public_async_test_workflow]",
      "[targets.api_parity_public_phase_b_surface_closure]",
  };
  for (std::size_t index = 0; index < public_example_paths.size(); ++index) {
    const std::string entry = std::string{"main = \""} +
        (public_example_paths[index] + sizeof("examples/api_parity/") - 1) + "\"";
    if (!contains(manifests, entry.c_str()) ||
        !contains(manifests, public_example_targets[index])) {
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
      "\"public_api_compatibility_examples_test.cpp\"",
      "\"public_api_example_expansion_test.cpp\"",
      "\"public_context_capability_example_test.cpp\"",
      "\"public_async_test_workflow_example_test.cpp\"",
      "\"public_phase_b_surface_closure_example_test.cpp\"",
      "\"public_authoring_vocabulary_freeze_test.cpp\"",
      "\"public_phase_b_completion_audit_test.cpp\"",
  };
  for (std::size_t index = 0; index < public_test_paths.size(); ++index) {
    if (read_source(public_test_paths[index]).empty() ||
        !contains(manifests, public_test_targets[index])) {
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
