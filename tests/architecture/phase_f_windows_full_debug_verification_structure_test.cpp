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
  const std::string child_guard = read_source(
      "tests/architecture/phase_f_child_window_ownership_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &app_context, &renderer_results, &win32_window, &win32_application,
      &wayland_lifecycle, &pointer_guard, &child_guard, &xmake, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;
  if (!contains(app_context, "create_child_window(") ||
      !contains(app_context, "child_window_callback") ||
      !contains(renderer_results, "run_callback") ||
      !contains(renderer_results, "factory_count != 2")) return 2;
  if (line_count(win32_window) > 120 || line_count(win32_application) > 170 ||
      line_count(wayland_lifecycle) > 25) return 3;
  if (!contains(pointer_guard, "runtime_window_input_state.cpp") ||
      !contains(pointer_guard, "pointer_position_for(event)") ||
      !contains(child_guard, "Step 630 Linux AT-SPI text and value event production behavior")) return 4;
  if (!contains(xmake, "target(\"phase_f_windows_full_debug_verification_structure_test\")")) return 5;
  constexpr const char* completion =
      "Phase F Step 611 completes Windows full-debug verification at 338/338 after restoring deferred child-window fixtures, child renderer result coverage, extracted pointer-input ownership, and existing source caps. Step 612 WSL full-debug verification is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 6;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 630 Linux AT-SPI text and value event production behavior\"")) return 7;
  return 0;
}
