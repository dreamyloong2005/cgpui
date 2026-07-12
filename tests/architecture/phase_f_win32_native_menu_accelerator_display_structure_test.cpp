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
  const std::string header = read_source(
      "src/platform/win32/win32_native_menu_accelerator_internal.hpp");
  const std::string source = read_source(
      "src/platform/win32/win32_native_menu_accelerator.cpp");
  const std::string tree = read_source(
      "src/platform/win32/win32_native_menu_tree.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_native_menu_accelerator_display_test.cpp");
  const std::string win32_structure = read_source(
      "tests/architecture/win32_window_source_test.cpp");
  const std::string platform_structure = read_source(
      "tests/architecture/platform_source_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &header, &source, &tree, &behavior, &win32_structure,
      &platform_structure, &xmake, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "win32_native_menu_display_title(") ||
      !contains(tree, "#include \"win32_native_menu_accelerator_internal.hpp\"") ||
      !contains(tree, "win32_native_menu_display_title(item)")) return 2;
  if (!contains(source, "VK_F1") || !contains(source, "VK_F24") ||
      !contains(source, "case VK_DELETE: return L\"Del\"") ||
      !contains(source, "title += L'\\t'") ||
      !contains(source, "L\"Ctrl\"") || !contains(source, "L\"Alt\"") ||
      !contains(source, "L\"Shift\"") || !contains(source, "L\"Win\"")) return 3;
  const auto control = source.find("modifiers.control");
  const auto alt = source.find("modifiers.alt", control);
  const auto shift = source.find("modifiers.shift", alt);
  const auto super = source.find("modifiers.super", shift);
  if (control == std::string::npos || alt == std::string::npos ||
      shift == std::string::npos || super == std::string::npos) return 4;
  if (!contains(behavior, "Open\\tCtrl+Shift+O") ||
      !contains(behavior, "Close\\tAlt+F4") ||
      !contains(behavior, "Delete\\tCtrl+Del") ||
      !contains(behavior, "Workspace 1\\tWin+1")) return 5;
  if (!contains(win32_structure, "win32_native_menu_accelerator_internal.hpp") ||
      !contains(win32_structure, "win32_native_menu_accelerator.cpp") ||
      !contains(platform_structure, "win32_native_menu_accelerator_internal.hpp") ||
      !contains(platform_structure, "win32_native_menu_accelerator.cpp")) return 6;
  if (line_count(header) > 25 || line_count(source) > 90 ||
      line_count(tree) > 125 || line_count(behavior) > 115) return 7;
  if (!contains(xmake, "target(\"win32_native_menu_accelerator_display_test\")") ||
      !contains(xmake, "target(\"phase_f_win32_native_menu_accelerator_display_structure_test\")")) return 8;
  constexpr const char* completion =
      "Phase F Step 582 formats Win32 native menu accelerator labels in a focused leaf, appends tab-aligned Ctrl/Alt/Shift/Win key text for letters, digits, function keys, and named keys, and preserves accelerator counts without claiming dispatch. Step 583 native menu accelerator registration production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 9;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 634 Linux AT-SPI production closeout audit")) return 10;
  return 0;
}
