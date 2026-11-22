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
  const std::string theme = read_source("src/ui/runtime_theme.cpp");
  const std::string scheduling = read_source("src/ui/runtime_theme_scheduling.cpp");
  const std::string scheduling_header = read_source("include/cgpui/ui/detail/window_runtime_theme_scheduling_declarations.hpp");
  const std::string window_scheduling = read_source("src/ui/runtime_window_scheduling.cpp");
  const std::string private_header = read_source("include/cgpui/ui/detail/window_runtime_declarations.hpp");
  const std::string context = read_source("src/ui/runtime_context_platform.cpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_theme_test.cpp");
  const std::string ui_structure = read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&theme, &scheduling, &scheduling_header, &window_scheduling,
      &private_header, &context, &behavior, &ui_structure, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(theme, "request_all_theme_renders()") ||
      !contains(theme, "request_theme_render(runtime_id)") ||
      contains(theme, "request_render();")) return 2;
  if (!contains(scheduling, "WindowRuntime::request_theme_render(") ||
      !contains(scheduling, "WindowRuntime::request_all_theme_renders(") ||
      !contains(scheduling, "request_render_for_window(runtime_id)") ||
      !contains(window_scheduling, "WindowRuntime::request_render_for_window(") ||
      !contains(window_scheduling, "request_paint_for_window(runtime_id)") ||
      !contains(window_scheduling, "find_window_runtime_record(runtime_id)") ||
      !contains(window_scheduling, "record->redraw_scheduled = true") ||
      !contains(window_scheduling, "record->window->request_redraw()")) return 3;
  if (!contains(context, "runtime.theme_color(window_runtime_id, id)") ||
      !contains(context, "runtime.theme_spacing(window_runtime_id, id)") ||
      !contains(context, "runtime.set_window_theme(window_runtime_id")) return 4;
  if (!contains(behavior, "app_redraws_all") ||
      !contains(behavior, "child_redraw_isolated") ||
      !contains(behavior, "root_redraw_isolated") ||
      !contains(behavior, "clear_redraw_isolated")) return 5;
  if (!contains(private_header, "window_runtime_theme_scheduling_declarations.hpp") ||
      !contains(ui_structure, "runtime_theme_scheduling_source")) return 6;
  if (line_count(theme) > 75 || line_count(scheduling) > 35 ||
      line_count(window_scheduling) > 90 ||
      line_count(scheduling_header) > 5 || line_count(private_header) > 260 ||
      line_count(context) > 70 || line_count(behavior) > 210) return 7;
  if (!contains(manifests, "\"window_runtime_multi_window_theme_test.cpp\"") ||
      !contains(manifests, "\"phase_f_multi_window_theme_isolation_structure_test.cpp\"")) return 8;
  return 0;
}
