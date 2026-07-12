#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}
bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}
std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}
} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string win32 = read_source("src/platform/win32/win32_application.cpp");
  const std::string wayland = read_source("src/platform/linux/wayland_application.cpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&xmake, &win32, &wayland, &roadmap, &ledger_md,
                                &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  constexpr std::array closeout_guards{
      "phase_f_child_window_ownership_structure_test",
      "phase_f_win32_input_dpi_structure_test",
      "phase_f_wayland_event_loop_wakeup_structure_test",
      "phase_f_wayland_clipboard_diagnostics_structure_test",
      "phase_f_drag_drop_closeout_structure_test",
      "phase_f_native_menu_closeout_structure_test",
      "phase_f_platform_services_closeout_structure_test",
      "phase_f_multi_window_event_loop_closeout_structure_test",
      "phase_f_platform_diagnostics_stress_closeout_structure_test",
      "phase_f_windows_full_debug_verification_structure_test",
      "phase_f_wsl_full_debug_verification_structure_test",
      "phase_f_cross_platform_test_execution_structure_test",
      "phase_f_platform_production_path_audit_test",
  };
  for (std::size_t index = 0; index < closeout_guards.size(); ++index) {
    if (!contains(xmake, closeout_guards[index])) return 10 + static_cast<int>(index);
  }
  if (line_count(win32) > 170 || line_count(wayland) > 140 ||
      !contains(win32, "int run() override") ||
      !contains(wayland, "WaylandApplication::WaylandApplication()")) return 30;

  constexpr const char* completion =
      "Phase F Step 614 audits the Win32/Wayland production path across lifecycle, input, clipboard, drag/drop, menus, services, multi-window ownership, diagnostics, and final host verification without widening platform entry files. Step 615 final closeout guard is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 31;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 618 Phase F final closeout\"")) return 32;
  return 0;
}
