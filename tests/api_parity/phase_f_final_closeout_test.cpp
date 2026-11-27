#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  std::string text = source
                         ? std::string{std::istreambuf_iterator<char>(source), {}}
                         : std::string{};
  text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
  return text;
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
  const std::string manifests = read_source("mcpp.toml") +
      read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string win32 = read_source("src/platform/win32/win32_application.cpp");
  const std::string wayland = read_source("src/platform/linux/wayland_application.cpp");
  const std::string* required[]{
      &manifests,
      &win32,
      &wayland};
  for (const auto* source : required) if (source->empty()) return 1;

  constexpr std::array closeout_targets{
      "phase_f_child_window_ownership_structure_test.cpp",
      "phase_f_win32_input_dpi_structure_test.cpp",
      "phase_f_wayland_event_loop_wakeup_structure_test.cpp",
      "phase_f_wayland_clipboard_diagnostics_structure_test.cpp",
      "phase_f_drag_drop_closeout_structure_test.cpp",
      "phase_f_native_menu_closeout_structure_test.cpp",
      "phase_f_platform_services_closeout_structure_test.cpp",
      "phase_f_multi_window_event_loop_closeout_structure_test.cpp",
      "phase_f_platform_diagnostics_stress_closeout_structure_test.cpp",
      "phase_f_windows_full_debug_verification_structure_test.cpp",
      "phase_f_wsl_full_debug_verification_structure_test.cpp",
      "phase_f_cross_platform_test_execution_structure_test.cpp",
      "phase_f_platform_production_path_audit_test.cpp",
      "phase_f_final_closeout_test.cpp",
  };
  for (std::size_t index = 0; index < closeout_targets.size(); ++index) {
    if (!contains(manifests, closeout_targets[index])) return 10 + static_cast<int>(index);
  }
  if (line_count(win32) > 170 || line_count(wayland) > 140) return 30;

  return 0;
}
