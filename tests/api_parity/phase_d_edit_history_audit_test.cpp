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

std::string line_containing(const std::string& text, const char* anchor) {
  const std::size_t anchor_index = text.find(anchor);
  if (anchor_index == std::string::npos) {
    return {};
  }
  const std::size_t line_start = text.rfind('\n', anchor_index);
  const std::size_t line_end = text.find('\n', anchor_index);
  const std::size_t start =
      line_start == std::string::npos ? 0 : line_start + 1U;
  const std::size_t end =
      line_end == std::string::npos ? text.size() : line_end;
  return text.substr(start, end - start);
}

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string text_model_header =
      read_source("include/cgpui/ui/text_model.hpp");
  const std::string text_model_source = read_source("src/ui/text_model.cpp");
  const std::string text_model_history =
      read_source("src/ui/text_model_history.cpp");
  const std::string runtime_clipboard =
      read_source("src/ui/runtime_clipboard.cpp");
  const std::string text_model_test =
      read_source("tests/ui/text_model_test.cpp");
  const std::string structure_test =
      read_source("tests/architecture/ui_source_structure_test.cpp");

  if (xmake.empty() || roadmap.empty() || ledger.empty() ||
      task_plan.empty() || findings.empty() || text_model_header.empty() ||
      text_model_source.empty() || text_model_history.empty() ||
      runtime_clipboard.empty() || text_model_test.empty() ||
      structure_test.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_d_edit_history_audit_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_d_edit_history_audit_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "- [x] Steps 419-426: Deepen edit history") ||
      contains(roadmap, "- [ ] Steps 419-426") ||
      !contains(roadmap,
                "Step 424 closes the edit-history band through") ||
      !contains(roadmap,
                "tests/api_parity/phase_d_edit_history_audit_test.cpp") ||
      !contains(roadmap, "Steps 427-434: Complete IME on active targets")) {
    return 3;
  }

  if (!contains(task_plan, "Step 424 closes the edit-history band") ||
      !contains(findings, "Step 424 is an audit-only closeout")) {
    return 4;
  }

  const std::string text_row = line_containing(ledger, "| gpui text system |");
  if (text_row.empty() || !contains(text_row, "Phase D Steps 379-424") ||
      !contains(text_row, "adjacent typing history coalescing") ||
      !contains(text_row, "composition-history grouping") ||
      !contains(text_row, "undo-manager integration points") ||
      !contains(text_row, "redo invalidation diagnostics") ||
      !contains(text_row, "edit transaction diagnostics") ||
      !contains(text_row, "Step 424 edit-history band audit") ||
      !contains(text_row, "phase_d_edit_history_audit_test") ||
      !contains(text_row, "deeper IME behavior")) {
    return 5;
  }

  constexpr std::array public_evidence{
      "enum class TextInsertHistoryPolicy",
      "merge_adjacent_typing",
      "composition_commit",
      "struct TextEditHistoryStatus",
      "struct TextEditHistoryRedoInvalidation",
      "enum class TextEditHistoryTransactionKind",
      "struct TextEditHistoryTransactionDiagnostic",
      "TextEditHistoryStatus edit_history_status()",
      "void mark_edit_history_clean()",
  };
  for (std::size_t index = 0; index < public_evidence.size(); ++index) {
    if (!contains(text_model_header, public_evidence[index])) {
      return 20 + static_cast<int>(index);
    }
  }

  constexpr std::array source_evidence{
      "TextModel::commit_history_record(",
      "TextModel::invalidate_redo_history(",
      "TextModel::record_edit_history_transaction(",
      "TextEditHistoryTransactionKind::record_merged",
      "TextEditHistoryRedoInvalidationReason::branch_edit",
      "TextInsertHistoryPolicy::composition_commit",
      "composition_history_mutated_ = true",
      "TextInsertHistoryPolicy::separate_edit",
  };
  const std::array source_files{
      text_model_history,
      text_model_history,
      text_model_history,
      text_model_history,
      text_model_history,
      text_model_source,
      text_model_source,
      runtime_clipboard,
  };
  for (std::size_t index = 0; index < source_evidence.size(); ++index) {
    if (!contains(source_files[index], source_evidence[index]) ||
        !contains(structure_test, source_evidence[index])) {
      return 40 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      "test_text_model_groups_adjacent_typing_history",
      "test_text_model_reports_edit_history_status_for_undo_manager",
      "test_text_model_reports_redo_invalidation_diagnostics",
      "test_text_model_reports_edit_history_transaction_diagnostics",
      "test_text_model_groups_ime_surrounding_delete_with_commit_history",
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    if (!contains(text_model_test, behavior_evidence[index])) {
      return 60 + static_cast<int>(index);
    }
  }

  return 0;
}
