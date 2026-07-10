#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }
  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char character : text) {
    if (character == '\n') {
      ++lines;
    }
  }
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string event_header =
      read_source("include/cgpui/core/event_window.hpp");
  const std::string close_header = read_source(
      "include/cgpui/platform/platform_window_close.hpp");
  const std::string platform_window = read_source(
      "include/cgpui/platform/platform_window.hpp");
  const std::string close_internal = read_source(
      "src/platform/platform_window_close_internal.hpp");
  const std::string close_source =
      read_source("src/platform/platform_window_close.cpp");
  const std::string win32_window =
      read_source("src/platform/win32/win32_window.cpp");
  const std::string win32_close =
      read_source("src/platform/win32/win32_window_close.cpp");
  const std::string win32_proc = read_source(
      "src/platform/win32/win32_window_proc_lifecycle.cpp");
  const std::string wayland_window =
      read_source("src/platform/linux/wayland_window.cpp");
  const std::string wayland_close =
      read_source("src/platform/linux/wayland_window_close.cpp");
  const std::string wayland_configure =
      read_source("src/platform/linux/wayland_window_configure.cpp");
  const std::string wayland_registered =
      read_source("src/platform/linux/wayland_window_registered.cpp");
  const std::string runtime_context =
      read_source("include/cgpui/ui/runtime_context.hpp");
  const std::string runtime_context_close =
      read_source("src/ui/runtime_context_window_close.cpp");
  const std::string runtime_control =
      read_source("src/ui/runtime_event_control.cpp");
  const std::string runtime_windows =
      read_source("src/ui/runtime_event_windows.cpp");
  const std::string app_runner_test =
      read_source("tests/ui/app_runner_test.cpp");
  const std::string win32_test =
      read_source("tests/platform/win32_window_close_policy_test.cpp");
  const std::string wayland_test =
      read_source("tests/platform/wayland_window_close_policy_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  if (event_header.empty() || close_header.empty() ||
      platform_window.empty() || close_internal.empty() ||
      close_source.empty() || win32_window.empty() || win32_close.empty() ||
      win32_proc.empty() || wayland_window.empty() || wayland_close.empty() ||
      wayland_configure.empty() || wayland_registered.empty() ||
      runtime_context.empty() || runtime_context_close.empty() ||
      runtime_control.empty() || runtime_windows.empty() ||
      app_runner_test.empty() || win32_test.empty() || wayland_test.empty() ||
      xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty()) {
    return 1;
  }

  if (!contains(event_header, "enum class WindowCloseRequestSource") ||
      !contains(event_header, "WindowCloseRequestSource source") ||
      !contains(event_header, "std::uint64_t sequence")) {
    return 2;
  }
  if (!contains(close_header, "PlatformWindowCloseResolution") ||
      !contains(close_header, "struct PlatformWindowCloseState") ||
      !contains(platform_window, "close_request_state() const") ||
      !contains(platform_window, "resolve_close_request(")) {
    return 3;
  }
  if (!contains(close_internal, "class PlatformWindowCloseController") ||
      !contains(close_source, "state_.coalesced_count") ||
      !contains(close_source, "state_.cancelled_count") ||
      !contains(close_source, "state_.accepted_count")) {
    return 4;
  }
  if (!contains(win32_close, "win32_application_close_wparam") ||
      !contains(win32_close, "close_controller_.begin(source)") ||
      !contains(win32_proc, "WindowCloseRequestSource::window_manager") ||
      contains(win32_window, "Win32Window::close_requested(")) {
    return 5;
  }
  if (!contains(wayland_close, "WindowCloseRequestSource::application") ||
      !contains(wayland_close, "close_controller_.begin(source)") ||
      !contains(wayland_configure,
                "WindowCloseRequestSource::window_manager") ||
      !contains(wayland_registered, "close_request_state() const") ||
      contains(wayland_window, "WaylandWindow::close_requested(")) {
    return 6;
  }
  if (!contains(runtime_context, "accept_window_close() const") ||
      !contains(runtime_context, "cancel_window_close() const") ||
      !contains(runtime_context_close,
                "WindowRuntimeContext::cancel_window_close() const") ||
      !contains(runtime_control, "close_request_state().pending") ||
      !contains(runtime_control, "close_request_state().accepted") ||
      !contains(runtime_windows, "PlatformWindowCloseResolution::accept")) {
    return 7;
  }
  if (!contains(app_runner_test,
                "test_runtime_close_callback_can_cancel_then_accept") ||
      !contains(win32_test, "cancelled.coalesced_count != 1") ||
      !contains(wayland_test, "cancelled.coalesced_count != 1")) {
    return 8;
  }
  if (!contains(xmake, "target(\"win32_window_close_policy_test\")") ||
      !contains(xmake, "target(\"wayland_window_close_policy_test\")") ||
      !contains(xmake,
                "target(\"phase_f_window_close_policy_structure_test\")")) {
    return 9;
  }
  if (line_count(close_header) > 35 || line_count(close_internal) > 25 ||
      line_count(close_source) > 70 || line_count(win32_close) > 45 ||
      line_count(wayland_close) > 45 ||
      line_count(runtime_context_close) > 25 ||
      line_count(runtime_control) > 75 || line_count(runtime_windows) > 115) {
    return 10;
  }

  constexpr const char* completion =
      "Phase F Step 542 adds source-aware, state-before-event close requests "
      "with shared pending/accept/cancel/coalescing policy on Win32 and "
      "Wayland, plus runtime callback cancellation with compatible default "
      "acceptance. Step 543 fullscreen and minimize/maximize production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 11;
    }
  }
  if (!contains(
          ledger_json,
          "\"phase_f_step_542_remaining_gap\": \"Step 543 fullscreen and minimize/maximize")) {
    return 12;
  }
  return 0;
}
