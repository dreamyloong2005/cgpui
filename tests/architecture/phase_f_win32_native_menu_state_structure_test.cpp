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
  const std::string public_menu = read_source(
      "include/cgpui/platform/platform_native_menu.hpp");
  const std::string source = read_source(
      "src/platform/win32/win32_native_menu_tree.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_native_menu_tree_test.cpp");
  const std::string tree_structure = read_source(
      "tests/architecture/phase_f_win32_native_menu_tree_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &public_menu, &source, &behavior, &tree_structure, &xmake, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_menu, "bool enabled = true") ||
      !contains(public_menu, "bool checked = false") ||
      !contains(public_menu, "bool radio = false") ||
      public_menu.find("bool radio = false") >
          public_menu.find("std::vector<NativeMenuItem> children")) return 2;
  if (!contains(source, "MENUITEMINFOW info") ||
      !contains(source, "MFT_RADIOCHECK") ||
      !contains(source, "MFS_GRAYED") ||
      !contains(source, "MFS_CHECKED") ||
      !contains(source, "InsertMenuItemW(")) return 3;
  if (!contains(behavior, ".enabled = false") ||
      !contains(behavior, ".checked = true") ||
      !contains(behavior, ".radio = true") ||
      !contains(behavior, "MIIM_FTYPE | MIIM_STATE") ||
      !contains(behavior, "MFT_RADIOCHECK") ||
      !contains(behavior, "MFS_CHECKED")) return 4;
  if (!contains(tree_structure, "line_count(source) > 125") ||
      !contains(tree_structure, "line_count(behavior) > 130")) return 5;
  if (line_count(public_menu) > 65 || line_count(source) > 125 ||
      line_count(behavior) > 130) return 6;
  if (!contains(xmake, "target(\"phase_f_win32_native_menu_state_structure_test\")") ||
      !contains(xmake, "target(\"win32_native_menu_tree_test\")")) return 7;
  constexpr const char* completion =
      "Phase F Step 580 maps enabled, checked, and radio menu state through structured Win32 MENUITEMINFO records, adds a defaulted public radio flag, and verifies disabled, checkmark, and radio-check rendering on a real HMENU. Step 581 native menu dynamic update production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 8;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 652 PNG and JPEG decode boundary production behavior")) return 9;
  return 0;
}
