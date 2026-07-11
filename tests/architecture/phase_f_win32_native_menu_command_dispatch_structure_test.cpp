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
  const std::string event = read_source("include/cgpui/core/event_native_menu.hpp");
  const std::string events = read_source("include/cgpui/core/event_platform.hpp");
  const std::string aggregate = read_source("include/cgpui/core/events.hpp");
  const std::string map_header = read_source(
      "src/platform/win32/win32_native_menu_command_internal.hpp");
  const std::string map_source = read_source(
      "src/platform/win32/win32_native_menu_command.cpp");
  const std::string window_source = read_source(
      "src/platform/win32/win32_window_native_menu.cpp");
  const std::string proc_header = read_source(
      "src/platform/win32/win32_window_proc_command_internal.hpp");
  const std::string proc_source = read_source(
      "src/platform/win32/win32_window_proc_command.cpp");
  const std::string proc = read_source("src/platform/win32/win32_window_proc.cpp");
  const std::string runtime = read_source("src/ui/runtime_event_native_menu.cpp");
  const std::string runtime_entry = read_source("src/ui/runtime_events.cpp");
  const std::string runtime_windows = read_source("src/ui/runtime_event_windows.cpp");
  const std::string platform_behavior = read_source(
      "tests/platform/win32_native_menu_command_test.cpp");
  const std::string runtime_behavior = read_source(
      "tests/ui/native_menu_action_dispatch_test.cpp");
  const std::string win32_structure = read_source(
      "tests/architecture/win32_window_source_test.cpp");
  const std::string platform_structure = read_source(
      "tests/architecture/platform_source_structure_test.cpp");
  const std::string ui_structure = read_source(
      "tests/architecture/ui_source_structure_test.cpp");
  const std::string renderer_structure = read_source(
      "tests/architecture/renderer_source_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &event, &events, &aggregate, &map_header, &map_source, &window_source,
      &proc_header, &proc_source, &proc, &runtime, &runtime_entry,
      &runtime_windows, &platform_behavior, &runtime_behavior, &win32_structure,
      &platform_structure, &ui_structure, &renderer_structure, &xmake, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(event, "enum class NativeMenuCommandSource") ||
      !contains(event, "struct NativeMenuCommand") ||
      !contains(events, "NativeMenuCommand,") ||
      !contains(events, "native_menu_command,") ||
      !contains(aggregate, "event_native_menu.hpp")) return 2;
  if (!contains(map_header, "class Win32NativeMenuCommandMap") ||
      !contains(map_header, "std::shared_ptr<const Win32NativeMenuCommandMap>") ||
      !contains(map_source, "append_command_actions(") ||
      !contains(map_source, "win32_native_menu_first_command_id") ||
      !contains(map_source, "action_names_[index]")) return 3;
  if (!contains(window_source, "native_menu_commands_->resolve(") ||
      !contains(window_source, "callback_(std::move(*command))") ||
      !contains(proc_header, "win32_window_proc_handle_command(") ||
      !contains(proc_source, "message != WM_COMMAND") ||
      !contains(proc_source, "NativeMenuCommandSource::accelerator") ||
      !contains(proc, "win32_window_proc_handle_command(")) return 4;
  if (!contains(runtime, "handle_native_menu_command_event(") ||
      !contains(runtime, "dispatch_action(command->action_name)") ||
      !contains(runtime, "finish_event_dispatch(dispatch.result)") ||
      !contains(runtime_entry, "handle_native_menu_command_event(event, root_view_id_)") ||
      !contains(runtime_windows, "handle_native_menu_command_event(event, record->root_view_id)")) return 5;
  if (!contains(platform_behavior, "MAKEWPARAM(0x1000U, 0U)") ||
      !contains(platform_behavior, "MAKEWPARAM(0x1000U, 1U)") ||
      !contains(runtime_behavior, "register_window_action(") ||
      !contains(runtime_behavior, "dispatch->scope != cgpui::ActionScope::window")) return 6;
  if (!contains(win32_structure, "win32_native_menu_command_internal.hpp") ||
      !contains(win32_structure, "win32_window_proc_command.cpp") ||
      !contains(platform_structure, "win32_window_native_menu.cpp") ||
      !contains(ui_structure, "runtime_event_native_menu.cpp") ||
      !contains(renderer_structure, "event_native_menu.hpp")) return 7;
  if (line_count(event) > 35 || line_count(map_header) > 45 ||
      line_count(map_source) > 80 || line_count(window_source) > 40 ||
      line_count(proc_source) > 45 || line_count(runtime) > 35 ||
      line_count(platform_behavior) > 85 || line_count(runtime_behavior) > 70) return 8;
  if (!contains(xmake, "target(\"win32_native_menu_command_test\")") ||
      !contains(xmake, "target(\"native_menu_action_dispatch_test\")") ||
      !contains(xmake, "target(\"phase_f_win32_native_menu_command_dispatch_structure_test\")")) return 9;
  constexpr const char* completion =
      "Phase F Step 584 maps recursive Win32 menu command ids to immutable action names, emits menu/accelerator command events from WM_COMMAND, and routes them through existing scoped runtime action dispatch for root and additional windows. Step 585 native menu dispatch diagnostics production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 10;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 597 multi-window input and focus isolation")) return 11;
  return 0;
}
