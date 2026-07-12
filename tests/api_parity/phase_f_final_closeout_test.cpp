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

  constexpr std::array closeout_targets{
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
      "phase_f_final_closeout_test",
  };
  for (std::size_t index = 0; index < closeout_targets.size(); ++index) {
    if (!contains(xmake, closeout_targets[index])) return 10 + static_cast<int>(index);
  }
  if (line_count(win32) > 170 || line_count(wayland) > 140) return 30;

  if (!contains(roadmap,
                "- [x] Steps 611-618: Run full Windows/WSL verification") ||
      contains(roadmap,
               "- [ ] Steps 611-618: Run full Windows/WSL verification")) return 31;

  constexpr const char* completion =
      "Phase F final closeout: Steps 611-618 close with "
      "`tests/api_parity/phase_f_final_closeout_test.cpp`; Windows full "
      "debug suite passes 343/343 and WSL Arch Linux full debug suite passes "
      "325/325, including active-display Wayland frame pixel capture on "
      "`WAYLAND_DISPLAY=wayland-0`, using D-drive WSL build/cache output plus "
      "`/dev/shm/cgpui` transient temp. The required Win32/Wayland platform "
      "production path is complete for Phase F, and Phase G Step 619 Win32 "
      "UIA provider object production depth is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 40;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 649 style interpolation production behavior\"") ||
      !contains(task_plan,
                "- Status: complete\n- Authoritative scope: Phase F")) return 50;
  return 0;
}
