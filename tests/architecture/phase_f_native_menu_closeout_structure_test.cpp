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
} // namespace

int main() {
  const std::string tree = read_source(
      "tests/architecture/phase_f_win32_native_menu_tree_structure_test.cpp");
  const std::string state = read_source(
      "tests/architecture/phase_f_win32_native_menu_state_structure_test.cpp");
  const std::string replacement = read_source(
      "tests/architecture/phase_f_win32_native_menu_update_structure_test.cpp");
  const std::string display = read_source(
      "tests/architecture/phase_f_win32_native_menu_accelerator_display_structure_test.cpp");
  const std::string registration = read_source(
      "tests/architecture/phase_f_win32_native_menu_accelerator_registration_structure_test.cpp");
  const std::string dispatch = read_source(
      "tests/architecture/phase_f_win32_native_menu_command_dispatch_structure_test.cpp");
  const std::string diagnostics = read_source(
      "tests/architecture/phase_f_native_menu_dispatch_diagnostics_structure_test.cpp");
  const std::string wayland = read_source("src/platform/linux/wayland_native.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&tree, &state, &replacement, &display,
      &registration, &dispatch, &diagnostics, &wayland, &xmake, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(tree, "Phase F Step 579") ||
      !contains(state, "Phase F Step 580") ||
      !contains(replacement, "Phase F Step 581") ||
      !contains(display, "Phase F Step 582") ||
      !contains(registration, "Phase F Step 583") ||
      !contains(dispatch, "Phase F Step 584") ||
      !contains(diagnostics, "Phase F Step 585")) return 2;
  if (!contains(wayland, ".supported = false") ||
      !contains(wayland, ".backend = \"wayland\"") ||
      !contains(wayland, ".skipped_accelerator_count = accelerator_count")) return 3;
  if (!contains(xmake, "target(\"phase_f_native_menu_closeout_structure_test\")") ||
      !contains(xmake, "target(\"phase_f_native_menu_dispatch_diagnostics_structure_test\")") ||
      !contains(xmake, "target(\"phase_f_win32_native_menu_tree_structure_test\")")) return 4;
  constexpr const char* completion =
      "Phase F Step 586 audits and closes the Steps 579-585 native-menu and accelerator band, freezing Win32 menu tree/state/replacement, accelerator display/registration, command dispatch, diagnostics, and explicit Wayland unsupported behavior. Step 587 open-file dialog production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 5;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 637 structured task group production behavior")) return 6;
  return 0;
}
