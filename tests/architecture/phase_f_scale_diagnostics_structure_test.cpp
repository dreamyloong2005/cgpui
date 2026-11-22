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
std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines;
}
} // namespace

int main() {
  const std::string event_header =
      read_source("include/cgpui/core/event_platform.hpp");
  const std::string diagnostic_header =
      read_source("include/cgpui/platform/platform_diagnostics.hpp");
  const std::string helper_header =
      read_source("src/ui/runtime_window_scale_diagnostics_internal.hpp");
  const std::string helper =
      read_source("src/ui/runtime_window_scale_diagnostics.cpp");
  const std::string root = read_source("src/ui/runtime_rendering.cpp");
  const std::string child = read_source("src/ui/runtime_event_windows.cpp");
  const std::string behavior =
      read_source("tests/ui/window_runtime_scale_diagnostics_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&event_header, &diagnostic_header,
      &helper_header, &helper, &root, &child, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(event_header, "window_resized") ||
      !contains(diagnostic_header, "window_scale") ||
      !contains(diagnostic_header, "Size framebuffer_size") ||
      !contains(diagnostic_header, "DpiScale scale")) return 2;
  if (!contains(helper_header, "window_scale_diagnostic(") ||
      !contains(helper, ".kind = PlatformDiagnosticKind::window_scale") ||
      !contains(helper, ".event_kind = EventKind::window_resized") ||
      !contains(helper, ".operation = \"resize-scale\"") ||
      !contains(helper, ".framebuffer_size = event.size") ||
      !contains(helper, ".scale = event.scale")) return 3;
  if (!contains(root, "window_scale_diagnostic(event, result.has_value())") ||
      !contains(child,
                "window_scale_diagnostic(*resized, resize_result.has_value())")) return 4;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "platform_diagnostics.size() != 32") ||
      !contains(behavior, "source_index = static_cast<int>(index) + 32") ||
      !contains(behavior, "event.framebuffer_size.width") ||
      !contains(behavior, "event.scale.value")) return 5;
  if (line_count(event_header) > 75 || line_count(diagnostic_header) > 50 ||
      line_count(helper_header) > 20 || line_count(helper) > 30 ||
      line_count(root) > 40 || line_count(child) > 105 ||
      line_count(behavior) > 65) return 6;
  if (!contains(manifests, "\"window_runtime_scale_diagnostics_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_scale_diagnostics_structure_test.cpp\"")) return 7;
  return 0;
}
