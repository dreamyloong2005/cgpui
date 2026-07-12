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
  const std::string header = read_source("src/platform/win32/win32_file_dialog_internal.hpp");
  const std::string plan = read_source("src/platform/win32/win32_file_dialog.cpp");
  const std::string save = read_source("src/platform/win32/win32_file_dialog_save.cpp");
  const std::string behavior = read_source("tests/platform/win32_file_save_dialog_plan_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_win32_file_open_dialog_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&header, &plan, &save, &behavior, &previous,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "show_win32_native_save_file_dialog(") ||
      !contains(header, "suggested_name")) return 2;
  if (!contains(plan, "FOS_OVERWRITEPROMPT") ||
      !contains(plan, "show_win32_native_save_file_dialog(options, *plan)")) return 3;
  if (!contains(save, "CLSID_FileSaveDialog") ||
      !contains(save, "SetFileName(") || !contains(save, "SetFileTypes(") ||
      !contains(save, "ERROR_CANCELLED") || !contains(save, "GetResult(") ||
      !contains(save, "result.paths.size() == 1")) return 4;
  if (!contains(behavior, "FOS_OVERWRITEPROMPT") ||
      !contains(behavior, "project.cgpui") ||
      !contains(behavior, "L\"*.cgpui\"")) return 5;
  if (line_count(header) > 55 || line_count(plan) > 160 ||
      line_count(save) > 90 || line_count(behavior) > 35) return 6;
  if (!contains(previous, "Phase F Step 587") ||
      !contains(xmake, "target(\"win32_file_save_dialog_plan_test\")") ||
      !contains(xmake, "target(\"phase_f_win32_file_save_dialog_structure_test\")")) return 7;
  constexpr const char* completion =
      "Phase F Step 588 implements Win32 save-file dialogs in a focused IFileSaveDialog leaf, preserves suggested names and filters, requests overwrite confirmation without file-must-exist, and returns one filesystem path on acceptance. Step 589 native directory-picker production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 8;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 614 platform production-path audit")) return 9;
  return 0;
}
