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
  const std::string example =
      read_source("examples/api_parity/public_text_input_examples/main.cpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string xmake = read_source("xmake.lua");

  if (example.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty() ||
      xmake.empty()) {
    return 1;
  }

  if (!contains(xmake,
                "target(\"api_parity_public_text_input_examples\")") ||
      !contains(
          xmake,
          "examples/api_parity/public_text_input_examples/main.cpp") ||
      !contains(xmake,
                "target(\"phase_d_text_input_public_examples_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_d_text_input_public_examples_test.cpp")) {
    return 2;
  }

  constexpr std::array required_example_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicTextInputExamplesView",
      "cgpui::IntoElement render(",
      "cgpui::Context<PublicTextInputExamplesView>&",
      "static_assert(cgpui::Render<PublicTextInputExamplesView>)",
      "cgpui::TextModel",
      "cgpui::text_input(",
      "cgpui::TextInsertHistoryPolicy::merge_adjacent_typing",
      "cgpui::TextInsertHistoryPolicy::composition_commit",
      "cgpui::TextEditHistoryStatus",
      "cgpui::TextEditAction::move_next_word",
      "word_selection_range_at(",
      "line_selection_range_at(",
      "set_composition_text(",
      "commit_composition(",
      "delete_surrounding_text(",
      "cgpui::ImeTextInputPlacement",
      "cgpui::ImeComposition",
      "cgpui::append_ime_default_preedit_style",
      "cgpui::ImeDeleteSurroundingText",
      "candidate_rect",
      "content_hint",
      "content_purpose",
      "CGPUI_RUN_PUBLIC_TEXT_INPUT_EXAMPLES",
  };
  for (std::size_t index = 0; index < required_example_fragments.size();
       ++index) {
    if (!contains(example, required_example_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase D Step 443 text input public examples",
      "examples/api_parity/public_text_input_examples/main.cpp",
      "api_parity_public_text_input_examples",
      "phase_d_text_input_public_examples_test.cpp",
      "Step 443 starts the text-input public examples band",
      "TextModel public editing",
      "IME placement and composition metadata",
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

  if (!contains(ledger_json,
                "\"step_443\": \"Phase D Step 443 text input public examples\"") ||
      !contains(roadmap,
                "- [ ] Steps 443-450: Add text input parity examples")) {
    return 80;
  }

  constexpr std::array forbidden_example_fragments{
      "#include \"cgpui/ui/",
      "#include \"cgpui/platform/",
      "#include \"cgpui/renderer/",
      "#include \"src/",
      "#include \"../",
      "WindowRuntime",
      "TextModel::begin_composition_history_group",
      "TextModel::record_edit_history_transaction",
      "ClipboardItem",
      "gpui::test",
      "TaskPriority",
      "StructuredTaskGroup",
      ".runtime",
      "runtime.",
      "window_runtime",
  };
  for (std::size_t index = 0; index < forbidden_example_fragments.size();
       ++index) {
    if (contains(example, forbidden_example_fragments[index])) {
      return 100 + static_cast<int>(index);
    }
  }

  return 0;
}
