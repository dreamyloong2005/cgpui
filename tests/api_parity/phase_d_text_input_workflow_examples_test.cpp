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
      read_source("examples/api_parity/public_text_input_workflow/main.cpp");
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

  if (!contains(xmake, "target(\"api_parity_public_text_input_workflow\")") ||
      !contains(xmake,
                "examples/api_parity/public_text_input_workflow/main.cpp") ||
      !contains(xmake,
                "target(\"phase_d_text_input_workflow_examples_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_d_text_input_workflow_examples_test.cpp")) {
    return 2;
  }

  constexpr std::array required_example_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicTextInputWorkflowView",
      "cgpui::IntoElement render(",
      "cgpui::Context<PublicTextInputWorkflowView>&",
      "static_assert(cgpui::Render<PublicTextInputWorkflowView>)",
      "cgpui::TextModel",
      "mark_edit_history_clean(",
      "edit_history_status(",
      "cgpui::TextEditHistoryStatus",
      "cgpui::TextEditHistoryRedoInvalidationReason::branch_edit",
      "cgpui::TextEditHistoryTransactionKind::undo_applied",
      "undo(",
      "redo(",
      "backspace(",
      "delete_forward(",
      "move_cursor_next_word(",
      "move_cursor_next_line(",
      "apply_edit_action(cgpui::TextEditAction::extend_line_end)",
      "line_count(",
      "line_index_at(",
      "line_start_offset(",
      "line_end_offset(",
      "selected_text(",
      "set_composition_text(",
      "cancel_composition(",
      "has_composition(",
      "CGPUI_RUN_PUBLIC_TEXT_INPUT_WORKFLOW",
  };
  for (std::size_t index = 0; index < required_example_fragments.size();
       ++index) {
    if (!contains(example, required_example_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase D Step 445 text input workflow public example",
      "examples/api_parity/public_text_input_workflow/main.cpp",
      "api_parity_public_text_input_workflow",
      "phase_d_text_input_workflow_examples_test.cpp",
      "Step 445 extends the text-input examples band",
      "undo/redo and redo invalidation",
      "line/word navigation and composition cancellation",
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

  if (!contains(
          ledger_json,
          "\"step_445\": \"Phase D Step 445 text input workflow public example\"")) {
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
