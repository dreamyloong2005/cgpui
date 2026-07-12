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
  const std::string tree = read_source(
      "src/platform/win32/win32_native_menu_tree.cpp");
  const std::string state = read_source("src/platform/win32/win32_native.cpp");
  const std::string application = read_source(
      "src/platform/win32/win32_application.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_native_menu_update_test.cpp");
  const std::string state_structure = read_source(
      "tests/architecture/phase_f_win32_native_menu_state_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &tree, &state, &application, &behavior, &state_structure, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(tree, "if (model.items.empty()) return Win32NativeMenuTree{}") ||
      !contains(tree, "std::optional<Win32NativeMenuTree>") ||
      !contains(state, "menu_tree_ = std::move(*menu_tree)")) return 2;
  if (!contains(application, "win32_apply_native_menu(window_handle(window), nullptr)") ||
      !contains(application, "for (Win32Window* window : windows_)") ||
      !contains(application, "apply_native_menu(window)") ||
      !contains(application, "apply_native_menu(window->get())")) return 3;
  if (!contains(behavior, "first_window") ||
      !contains(behavior, "second_window") ||
      !contains(behavior, "updated_root == initial_root") ||
      !contains(behavior, "install_native_menu({})") ||
      !contains(behavior, "GetMenu(first_hwnd) != nullptr") ||
      !contains(behavior, "future_window")) return 4;
  if (!contains(state_structure, "MFT_RADIOCHECK") ||
      !contains(xmake, "target(\"win32_native_menu_replacement_test\")") ||
      !contains(xmake, "target(\"phase_f_win32_native_menu_replacement_structure_test\")")) return 5;
  if (line_count(tree) > 125 || line_count(state) > 70 ||
      line_count(application) > 190 || line_count(behavior) > 130) return 6;
  constexpr const char* completion =
      "Phase F Step 581 makes Win32 native menu installation dynamically replace all live window menus, preserves transactional tree ownership, treats an empty model as a successful clear, and keeps future windows aligned with the current menu state. Step 582 native menu accelerator display production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 7;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 632 Linux AT-SPI accessibility bus discovery and connection production behavior")) return 8;
  return 0;
}
