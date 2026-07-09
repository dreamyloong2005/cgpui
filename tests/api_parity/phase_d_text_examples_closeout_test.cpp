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
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");

  if (xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_d_text_examples_closeout_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_d_text_examples_closeout_test.cpp")) {
    return 2;
  }

  constexpr std::array example_targets{
      "target(\"api_parity_public_text_input_examples\")",
      "target(\"api_parity_public_text_wrapper_examples\")",
      "target(\"api_parity_public_text_input_workflow\")",
      "target(\"api_parity_public_rich_text_examples\")",
      "target(\"phase_d_text_input_public_examples_test\")",
      "target(\"phase_d_text_wrapper_public_examples_test\")",
      "target(\"phase_d_text_input_workflow_examples_test\")",
      "target(\"phase_d_rich_text_public_examples_test\")",
  };
  for (std::size_t index = 0; index < example_targets.size(); ++index) {
    if (!contains(xmake, example_targets[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase D text examples closeout",
      "phase_d_text_examples_closeout_test.cpp",
      "Step 450 closes the text examples band",
      "public-only prelude examples",
      "api_parity_public_text_input_examples",
      "api_parity_public_text_wrapper_examples",
      "api_parity_public_text_input_workflow",
      "api_parity_public_rich_text_examples",
      "renderer glyph coloring and inline image drawing remain later work",
  };
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
        !contains(ledger_md, required_docs[index]) ||
        !contains(ledger_json, required_docs[index]) ||
        !contains(task_plan, required_docs[index]) ||
        !contains(findings, required_docs[index])) {
      return 40 + static_cast<int>(index);
    }
  }

  if (!contains(roadmap,
                "- [x] Steps 443-450: Add text input parity examples")) {
    return 80;
  }

  return 0;
}
