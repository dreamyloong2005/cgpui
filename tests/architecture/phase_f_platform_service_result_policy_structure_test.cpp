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
  const std::string runtime_header = read_source("include/cgpui/ui/window_runtime.hpp");
  const std::string context_header = read_source("include/cgpui/ui/runtime_app_context.hpp");
  const std::string diagnostics = read_source("include/cgpui/platform/platform_diagnostics.hpp");
  const std::string source = read_source("src/ui/runtime_platform_service_system_results.cpp");
  const std::string state = read_source("src/ui/runtime_platform_service_state_internal.hpp");
  const std::string internal = read_source("src/ui/window_runtime_internal.hpp");
  const std::string context_source = read_source("src/ui/app_context_services.cpp");
  const std::string behavior = read_source("tests/api_parity/phase_f_platform_service_result_policy_test.cpp");
  const std::string ui_structure = read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_quit_reopen_lifecycle_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&runtime_header, &context_header, &diagnostics,
      &source, &state, &internal, &context_source, &behavior, &ui_structure,
      &previous, &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan,
      &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(runtime_header, "try_show_native_message_dialog(") ||
      !contains(runtime_header, "try_open_url(std::string url)") ||
      !contains(runtime_header, "try_request_reopen()") ||
      !contains(runtime_header, "native_message_dialog_result() const") ||
      !contains(runtime_header, "open_url_result() const") ||
      !contains(runtime_header, "reopen_result() const")) return 2;
  if (!contains(context_header, "try_show_native_message_dialog(") ||
      !contains(context_header, "try_open_url(") ||
      !contains(context_header, "try_request_reopen()") ||
      !contains(context_source, "AppContext::try_show_native_message_dialog(") ||
      !contains(context_source, "AppContext::try_open_url(") ||
      !contains(context_source, "AppContext::try_request_reopen()")) return 3;
  if (!contains(diagnostics, "message_dialog,") ||
      !contains(diagnostics, "open_url,") ||
      !contains(diagnostics, "reopen,")) return 4;
  if (!contains(source, "WindowRuntime::try_show_native_message_dialog(") ||
      !contains(source, "WindowRuntime::try_open_url(") ||
      !contains(source, "WindowRuntime::try_request_reopen()") ||
      !contains(source, "ErrorCode::unsupported_platform") ||
      !contains(source, "record_platform_diagnostic(")) return 5;
  if (!contains(state, "NativeMessageDialogResult native_message_dialog_result_") ||
      !contains(state, "PlatformOpenUrlResult open_url_result_") ||
      !contains(state, "PlatformReopenResult reopen_result_") ||
      !contains(internal, "#include \"runtime_platform_service_state_internal.hpp\"")) return 6;
  if (!contains(behavior, "bounded.size() != 32") ||
      !contains(behavior, "supported_incomplete_results_remain_values") ||
      !contains(behavior, "unsupported_attempts_preserve_last_supported_results") ||
      !contains(ui_structure, "runtime_platform_service_system_results.cpp") ||
      !contains(ui_structure, "runtime_platform_service_state_internal.hpp") ||
      !contains(ui_structure, "line_count(window_runtime_header) > 260")) return 7;
  if (contains(source, "MessageBox") || contains(source, "ShellExecute") ||
      contains(source, "HWND") || contains(source, "wl_display_") ||
      line_count(source) > 120 || line_count(state) > 10 ||
      line_count(runtime_header) > 260 || line_count(context_header) > 90 ||
      line_count(context_source) > 80 || line_count(behavior) > 250) return 8;
  if (!contains(previous, "Phase F Step 592") ||
      !contains(xmake, "target(\"phase_f_platform_service_result_policy_test\")") ||
      !contains(xmake, "target(\"phase_f_platform_service_result_policy_structure_test\")")) return 9;
  constexpr const char* completion =
      "Phase F Step 593 adds Runtime and AppContext Result adapters for native message dialogs, URL opening, and reopen requests, records typed bounded diagnostics, preserves supported incomplete results, and keeps the last supported service state across unsupported attempts. Step 594 dialogs and platform-services closeout audit is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 10;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Phase I Step 759 X11/XCB platform boundary")) return 11;
  return 0;
}
