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
      "src/platform/win32/win32_native_menu_tree_internal.hpp");
  const std::string source = read_source(
      "src/platform/win32/win32_native_menu_tree.cpp");
  const std::string state = read_source("src/platform/win32/win32_native.cpp");
  const std::string state_header = read_source(
      "src/platform/win32/win32_native_internal.hpp");
  const std::string application = read_source(
      "src/platform/win32/win32_application.cpp");
  const std::string wayland = read_source("src/platform/linux/wayland_native.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_native_menu_tree_test.cpp");
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
      &header, &source, &state, &state_header, &application, &wayland,
      &behavior, &win32_structure, &platform_structure, &xmake, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "class Win32NativeMenuTree") ||
      !contains(header, "std::optional<Win32NativeMenuTree> build(") ||
      !contains(header, "Win32NativeMenuTree(const Win32NativeMenuTree&) = delete") ||
      !contains(header, "win32_apply_native_menu(HWND hwnd, HMENU menu)")) return 2;
  if (!contains(source, "CreateMenu()") ||
      !contains(source, "CreatePopupMenu()") ||
      !contains(source, "append_menu_items(submenu") ||
      !contains(source, "InsertMenuItemW(") ||
      !contains(source, "DestroyMenu(root_)") ||
      !contains(source, "SetMenu(hwnd, menu)")) return 3;
  if (!contains(state_header, "Win32NativeMenuTree menu_tree_") ||
      !contains(state, "Win32NativeMenuTree::build(menu)") ||
      !contains(state, ".supported = menu_tree.has_value()") ||
      !contains(state, "menu_tree_ = std::move(*menu_tree)")) return 4;
  if (!contains(application, "win32_apply_native_menu(window_handle(window), nullptr)") ||
      !contains(application, "apply_native_menu(window->get())") ||
      !contains(application, "native_menu_state_.native_menu() == nullptr") ||
      !contains(application, "native_menu_state_.native_menu()")) return 5;
  if (!contains(behavior, "GetMenu(static_cast<HWND>") ||
      !contains(behavior, "GetSubMenu(root, 0)") ||
      !contains(behavior, "MF_SEPARATOR") ||
      !contains(behavior, "future_window") ||
      !contains(behavior, "L\"\\x6587\\x4EF6\"") ||
      !contains(wayland, ".supported = false")) return 6;
  if (!contains(win32_structure, "win32_native_menu_tree_internal.hpp") ||
      !contains(win32_structure, "win32_native_menu_tree.cpp") ||
      !contains(platform_structure, "win32_native_menu_tree_internal.hpp") ||
      !contains(platform_structure, "win32_native_menu_tree.cpp")) return 7;
  if (line_count(header) > 50 || line_count(source) > 125 ||
      line_count(state) > 70 || line_count(application) > 190 ||
      line_count(behavior) > 130) return 8;
  if (!contains(xmake, "target(\"win32_native_menu_tree_test\")") ||
      !contains(xmake, "target(\"phase_f_win32_native_menu_tree_structure_test\")")) return 9;
  constexpr const char* completion =
      "Phase F Step 579 builds and owns recursive Win32 HMENU trees in a focused leaf, attaches installed menus to existing and future windows, preserves nested Unicode titles and separators, and keeps Wayland explicitly unsupported. Step 580 native menu check/radio/enabled state production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 10;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Phase G Step 619 Win32 UIA provider object production depth")) return 11;
  return 0;
}
