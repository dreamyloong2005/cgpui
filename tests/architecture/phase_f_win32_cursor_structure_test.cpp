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
  const std::string pointer_header =
      read_source("include/cgpui/core/event_pointer.hpp");
  const std::string cursor_header =
      read_source("src/platform/win32/win32_cursor_internal.hpp");
  const std::string cursor =
      read_source("src/platform/win32/win32_cursor.cpp");
  const std::string window_cursor =
      read_source("src/platform/win32/win32_window_cursor.cpp");
  const std::string proc =
      read_source("src/platform/win32/win32_window_proc_cursor.cpp");
  const std::string proc_header = read_source(
      "src/platform/win32/win32_window_proc_cursor_internal.hpp");
  const std::string main_proc =
      read_source("src/platform/win32/win32_window_proc.cpp");
  const std::string window =
      read_source("src/platform/win32/win32_window.cpp");
  const std::string input_helpers =
      read_source("src/platform/win32/win32_input_helpers.cpp");
  const std::string window_internal =
      read_source("src/platform/win32/win32_window_internal.hpp");
  const std::string behavior =
      read_source("tests/platform/win32_cursor_production_test.cpp");
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
      &pointer_header, &cursor_header, &cursor, &window_cursor, &proc_header, &proc,
      &main_proc, &window, &input_helpers, &window_internal, &behavior, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(pointer_header, "resize_north_west_south_east") ||
      !contains(pointer_header, "resize_north_east_south_west") ||
      !contains(pointer_header, "resize_all") ||
      !contains(pointer_header, "progress") ||
      !contains(cursor_header, "load_win32_system_cursor(") ||
      !contains(cursor, "MAKEINTRESOURCEW(32642)") ||
      !contains(cursor, "MAKEINTRESOURCEW(32650)")) {
    return 2;
  }
  if (!contains(window_cursor, "Win32Window::set_cursor(") ||
      !contains(window_cursor, "Win32Window::refresh_cursor(") ||
      !contains(proc_header, "win32_window_proc_handle_cursor(") ||
      !contains(proc, "WM_SETCURSOR") || !contains(proc, "WM_THEMECHANGED") ||
      !contains(proc, "SPI_SETCURSORS") ||
      !contains(main_proc, "win32_window_proc_handle_cursor(") ||
      contains(window, "Win32Window::set_cursor(") ||
      contains(input_helpers, "cursor_id_for(") ||
      !contains(window_internal, "current_cursor_shape_")) {
    return 3;
  }
  if (!contains(behavior, "WM_SETCURSOR") ||
      !contains(behavior, "WM_SETTINGCHANGE") ||
      !contains(behavior, "CursorShape::resize_all") ||
      !contains(xmake, "target(\"win32_cursor_production_test\")") ||
      !contains(xmake, "target(\"phase_f_win32_cursor_structure_test\")")) {
    return 4;
  }
  if (line_count(pointer_header) > 60 || line_count(cursor_header) > 20 ||
      line_count(cursor) > 65 || line_count(window_cursor) > 40 ||
      line_count(proc_header) > 20 || line_count(proc) > 40 ||
      line_count(window_internal) > 120 ||
      line_count(behavior) > 85) {
    return 5;
  }
  constexpr const char* completion =
      "Phase F Step 552 adds production Win32 system cursors with expanded "
      "public shapes, focused system-resource mapping, client WM_SETCURSOR "
      "reapplication, and settings/theme refresh. Step 553 Win32 pointer "
      "capture and drag production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 6;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_552_remaining_gap\": \"Step 553 Win32")) {
    return 7;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 640 async timer integration production behavior")) {
    return 8;
  }
  return 0;
}
