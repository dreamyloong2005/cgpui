#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  return lines;
}
} // namespace

int main() {
  const std::string app_context =
      read_source("tests/api_parity/app_window_context_test.cpp");
  const std::string renderer_results =
      read_source("tests/api_parity/renderer_result_conventions_test.cpp");
  const std::string win32_window =
      read_source("src/platform/win32/win32_window_internal.hpp");
  const std::string win32_application =
      read_source("src/platform/win32/win32_application.cpp");
  const std::string wayland_lifecycle =
      read_source("src/platform/linux/wayland_application_lifecycle.cpp");
  const std::string pointer_guard = read_source(
      "tests/architecture/phase_f_wayland_pointer_motion_structure_test.cpp");
  const std::string manifests = read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &app_context, &renderer_results, &win32_window, &win32_application,
      &wayland_lifecycle, &pointer_guard, &manifests};
  for (const auto* source : required) if (source->empty()) return 1;
  if (!contains(app_context, "create_child_window(") ||
      !contains(app_context, "child_window_callback") ||
      !contains(renderer_results, "run_callback") ||
      !contains(renderer_results, "factory_count != 2")) return 2;
  if (line_count(win32_window) > 134 || line_count(win32_application) > 170 ||
      line_count(wayland_lifecycle) > 25) return 3;
  if (!contains(pointer_guard, "runtime_window_input_state.cpp") ||
      !contains(pointer_guard, "pointer_position_for(event)")) return 4;
  if (!contains(manifests, "\"phase_f_windows_full_debug_verification_structure_test.cpp\"")) return 5;
  return 0;
}
