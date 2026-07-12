#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}
bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}
std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines;
}
} // namespace

int main() {
  const std::string public_header = read_source("include/cgpui/platform/platform_file_dialog.hpp");
  const std::string plan = read_source("src/platform/win32/win32_file_dialog.cpp");
  const std::string behavior = read_source("tests/platform/win32_directory_dialog_plan_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_win32_file_save_dialog_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&public_header, &plan, &behavior, &previous,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "pick_directory,")) return 2;
  if (!contains(plan, "NativeFileDialogKind::pick_directory") ||
      !contains(plan, "FOS_PICKFOLDERS | FOS_PATHMUSTEXIST") ||
      !contains(plan, "if (options.kind == NativeFileDialogKind::pick_directory) break")) return 3;
  if (!contains(behavior, "FOS_PICKFOLDERS") ||
      !contains(behavior, "FOS_PATHMUSTEXIST") ||
      !contains(behavior, "!directory->filters.empty()")) return 4;
  if (line_count(public_header) > 45 || line_count(plan) > 160 ||
      line_count(behavior) > 35) return 5;
  if (!contains(previous, "Phase F Step 588") ||
      !contains(xmake, "target(\"win32_directory_dialog_plan_test\")") ||
      !contains(xmake, "target(\"phase_f_native_directory_picker_structure_test\")")) return 6;
  constexpr const char* completion =
      "Phase F Step 589 adds an explicit native directory-picker request kind, maps Win32 folder selection to FOS_PICKFOLDERS and existing-path requirements, ignores file filters, and preserves the shared single-path result contract. Step 590 native message-dialog production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 7;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 613 cross-platform test execution audit")) return 8;
  return 0;
}
