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
  const std::string menu_header = read_source(
      "include/cgpui/platform/platform_native_menu.hpp");
  const std::string win32_native = read_source(
      "src/platform/win32/win32_native.cpp");
  const std::string accelerator_table = read_source(
      "src/platform/win32/win32_native_menu_accelerator_table.cpp");
  const std::string empty = read_source("src/platform/empty.cpp");
  const std::string wayland = read_source("src/platform/linux/wayland_native.cpp");
  const std::string runtime = read_source("src/ui/runtime_event_native_menu.cpp");
  const std::string registration_behavior = read_source(
      "tests/platform/win32_native_menu_accelerator_registration_test.cpp");
  const std::string diagnostic_behavior = read_source(
      "tests/ui/native_menu_dispatch_diagnostics_test.cpp");
  const std::string previous_structure = read_source(
      "tests/architecture/phase_f_win32_native_menu_command_dispatch_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &menu_header, &win32_native, &accelerator_table, &empty, &wayland,
      &runtime, &registration_behavior, &diagnostic_behavior,
      &previous_structure, &xmake, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(menu_header, "registered_accelerator_count") ||
      !contains(menu_header, "skipped_accelerator_count")) return 2;
  if (!contains(accelerator_table, "registered_count() const") ||
      !contains(win32_native, "accelerator_table->registered_count()") ||
      !contains(win32_native, "accelerator_count - registered_accelerator_count") ||
      !contains(empty, ".skipped_accelerator_count = accelerator_count") ||
      !contains(wayland, ".skipped_accelerator_count = accelerator_count")) return 3;
  if (!contains(runtime, ".kind = PlatformDiagnosticKind::menu") ||
      !contains(runtime, ".event_kind = EventKind::native_menu_command") ||
      !contains(runtime, "dispatch-native-menu-accelerator") ||
      !contains(runtime, "dispatch-native-menu-command") ||
      !contains(runtime, ".succeeded = dispatch.handled") ||
      !contains(runtime, ".value_count = command->command_id")) return 4;
  if (!contains(registration_behavior, "registered_accelerator_count != 1") ||
      !contains(registration_behavior, "skipped_accelerator_count != 2") ||
      !contains(diagnostic_behavior, "diagnostics.size() != 2") ||
      !contains(diagnostic_behavior, "EventKind::native_menu_command") ||
      !contains(diagnostic_behavior, "diagnostics[1].succeeded")) return 5;
  if (line_count(menu_header) > 80 || line_count(win32_native) > 100 ||
      line_count(runtime) > 50 || line_count(registration_behavior) > 125 ||
      line_count(diagnostic_behavior) > 75) return 6;
  if (!contains(xmake, "target(\"native_menu_dispatch_diagnostics_test\")") ||
      !contains(xmake, "target(\"phase_f_native_menu_dispatch_diagnostics_structure_test\")")) return 7;
  constexpr const char* completion =
      "Phase F Step 585 reports declared, registered, and skipped native-menu accelerators across platform results, and records handled/unhandled menu command diagnostics with source, event kind, and command id. Step 586 native menu and accelerator closeout audit is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 8;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 668 Linux debug build and packaging coverage")) return 9;
  return 0;
}
