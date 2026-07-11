#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) {
    source.open(path);
  }
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) {
    lines += value == '\n' ? 1U : 0U;
  }
  return lines;
}

} // namespace

int main() {
  const std::string pointer_event =
      read_source("include/cgpui/core/event_pointer.hpp");
  const std::string event_platform =
      read_source("include/cgpui/core/event_platform.hpp");
  const std::string capture_header = read_source(
      "include/cgpui/platform/platform_window_pointer_capture.hpp");
  const std::string platform_window =
      read_source("include/cgpui/platform/platform_window.hpp");
  const std::string capture_default =
      read_source("src/platform/platform_window_pointer_capture.cpp");
  const std::string capture_internal = read_source(
      "src/platform/win32/win32_pointer_capture_internal.hpp");
  const std::string capture =
      read_source("src/platform/win32/win32_pointer_capture.cpp");
  const std::string window_capture = read_source(
      "src/platform/win32/win32_window_pointer_capture.cpp");
  const std::string proc_header = read_source(
      "src/platform/win32/win32_window_proc_pointer_capture_internal.hpp");
  const std::string proc = read_source(
      "src/platform/win32/win32_window_proc_pointer_capture.cpp");
  const std::string main_proc =
      read_source("src/platform/win32/win32_window_proc.cpp");
  const std::string window_internal =
      read_source("src/platform/win32/win32_window_internal.hpp");
  const std::string message_target =
      read_source("src/platform/win32/win32_window_message_internal.hpp");
  const std::string runtime_capture =
      read_source("src/ui/runtime_pointer_capture.cpp");
  const std::string runtime_input =
      read_source("src/ui/runtime_event_input.cpp");
  const std::string runtime_disabled =
      read_source("src/ui/runtime_disabled_interaction.cpp");
  const std::string behavior =
      read_source("tests/platform/win32_pointer_capture_test.cpp");
  const std::string runtime_behavior =
      read_source("tests/ui/window_runtime_pointer_capture_platform_test.cpp");
  const std::string source_inventory =
      read_source("tests/architecture/win32_window_source_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &pointer_event, &event_platform, &capture_header, &platform_window,
      &capture_default, &capture_internal, &capture, &window_capture,
      &proc_header, &proc, &main_proc, &window_internal, &message_target,
      &runtime_capture, &runtime_input, &runtime_disabled, &behavior,
      &runtime_behavior, &source_inventory, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(pointer_event, "struct PointerCaptureChanged") ||
      !contains(event_platform, "PointerCaptureChanged") ||
      !contains(event_platform, "pointer_capture_changed") ||
      !contains(capture_header, "struct PlatformPointerCaptureState") ||
      !contains(platform_window, "pointer_capture_state() const") ||
      !contains(platform_window, "set_pointer_capture(bool captured)")) {
    return 2;
  }
  if (!contains(capture_default, "PlatformWindow::pointer_capture_state()") ||
      !contains(capture_default, "PlatformWindow::set_pointer_capture(") ||
      !contains(capture, "SetCapture(hwnd)") ||
      !contains(capture, "GetCapture() == hwnd") ||
      !contains(capture, "ReleaseCapture()") ||
      !contains(window_capture, "Win32Window::set_pointer_capture(") ||
      !contains(window_capture, "Win32Window::pointer_capture_lost(")) {
    return 3;
  }
  if (!contains(proc, "WM_CAPTURECHANGED") ||
      !contains(proc, "WM_CANCELMODE") ||
      !contains(proc_header, "win32_window_proc_handle_pointer_capture(") ||
      !contains(main_proc, "win32_window_proc_handle_pointer_capture(") ||
      !contains(message_target, "pointer_capture_lost()") ||
      !contains(window_internal, "pointer_capture_state() const override")) {
    return 4;
  }
  if (!contains(runtime_capture, "window_->set_pointer_capture(true)") ||
      !contains(runtime_capture, "window_->set_pointer_capture(false)") ||
      !contains(runtime_capture, "input_.dragging = false") ||
      !contains(runtime_input, "PointerCaptureChanged") ||
      !contains(window_capture, "suppress_pointer_capture_lost_") ||
      contains(runtime_disabled, "pointer_capture_owner_.reset()")) {
    return 5;
  }
  if (!contains(behavior, "WM_CAPTURECHANGED") &&
      !contains(behavior, "SetCapture(other)")) {
    return 6;
  }
  if (!contains(behavior, "WM_CANCELMODE") ||
      !contains(runtime_behavior, "PointerCaptureChanged") ||
      !contains(source_inventory, "win32_pointer_capture.cpp") ||
      !contains(xmake, "target(\"win32_pointer_capture_test\")") ||
      !contains(xmake,
                "target(\"window_runtime_pointer_capture_platform_test\")") ||
      !contains(xmake,
                "target(\"phase_f_win32_pointer_capture_structure_test\")")) {
    return 7;
  }
  if (line_count(capture_header) > 25 || line_count(capture_internal) > 20 ||
      line_count(capture) > 35 || line_count(window_capture) > 45 ||
      line_count(proc_header) > 20 || line_count(proc) > 50 ||
      line_count(window_internal) > 120 || line_count(message_target) > 65 ||
      line_count(behavior) > 110) {
    return 8;
  }
  constexpr const char* completion =
      "Phase F Step 553 adds native Win32 pointer capture with compatible "
      "platform control, owner-matched release, capture-loss cancellation, "
      "and continuous outside-window drag movement. Step 554 Win32 input "
      "DPI-change production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 9;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_553_remaining_gap\": \"Step 554 Win32 input DPI-change")) {
    return 10;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 579 native menu")) {
    return 11;
  }
  return 0;
}
