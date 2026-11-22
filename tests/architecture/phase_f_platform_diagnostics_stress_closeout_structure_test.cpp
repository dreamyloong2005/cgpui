#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}
bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}
} // namespace

int main() {
  const char* guards[]{
      "tests/architecture/phase_f_window_churn_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_clipboard_ownership_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_drag_drop_cancellation_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_ime_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_scale_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_timer_wakeup_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_task_wakeup_diagnostics_structure_test.cpp",
  };
  for (const char* guard : guards) {
    if (read_source(guard).empty()) return 1;
  }
  const std::string manifests = read_source("mcpp.toml") +
      read_source("tests/architecture/mcpp.toml") +
      read_source("tests/platform/mcpp.toml") +
      read_source("tests/ui/mcpp.toml");
  const std::string* required[]{
      &manifests};
  for (const auto* value : required) if (value->empty()) return 2;
  const char* behavior_targets[]{
      "window_runtime_multi_window_churn_test.cpp",
      "wayland_clipboard_ownership_test.cpp",
      "window_runtime_drag_drop_cancellation_diagnostics_test.cpp",
      "window_runtime_ime_diagnostics_test.cpp",
      "window_runtime_scale_diagnostics_test.cpp",
      "window_runtime_timer_wakeup_diagnostics_test.cpp",
      "window_runtime_task_wakeup_diagnostics_test.cpp",
  };
  for (const char* target : behavior_targets) {
    if (!contains(manifests, target)) return 3;
  }
  const char* structure_targets[]{
      "phase_f_window_churn_diagnostics_structure_test.cpp",
      "phase_f_clipboard_ownership_diagnostics_structure_test.cpp",
      "phase_f_drag_drop_cancellation_diagnostics_structure_test.cpp",
      "phase_f_ime_diagnostics_structure_test.cpp",
      "phase_f_scale_diagnostics_structure_test.cpp",
      "phase_f_timer_wakeup_diagnostics_structure_test.cpp",
      "phase_f_task_wakeup_diagnostics_structure_test.cpp",
      "phase_f_platform_diagnostics_stress_closeout_structure_test.cpp",
  };
  for (const char* target : structure_targets) {
    if (!contains(manifests, target)) return 4;
  }
  return 0;
}
