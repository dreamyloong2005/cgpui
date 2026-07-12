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
  const std::string coordinates_header = read_source(
      "src/platform/win32/win32_input_coordinates_internal.hpp");
  const std::string coordinates =
      read_source("src/platform/win32/win32_input_coordinates.cpp");
  const std::string events =
      read_source("src/platform/win32/win32_window_pointer_events.cpp");
  const std::string drag =
      read_source("src/platform/win32/win32_window_drag_drop.cpp");
  const std::string size =
      read_source("src/platform/win32/win32_window_size.cpp");
  const std::string behavior =
      read_source("tests/platform/win32_input_dpi_test.cpp");
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
      &coordinates_header, &coordinates, &events, &drag, &size, &behavior,
      &source_inventory, &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan,
      &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(coordinates_header, "win32_logical_client_point(") ||
      !contains(coordinates_header, "win32_logical_client_point_from_screen(") ||
      !contains(coordinates, "scale.value") ||
      !contains(coordinates, "ScreenToClient") ||
      !contains(coordinates, "GET_X_LPARAM")) {
    return 2;
  }
  if (!contains(events, "win32_logical_client_point(") ||
      !contains(events, "win32_logical_client_point_from_screen(") ||
      contains(events, "static_cast<float>(GET_X_LPARAM") ||
      !contains(drag, "win32_logical_client_point_from_screen(") ||
      !contains(size, "state_.scale = DpiScale")) {
    return 3;
  }
  if (!contains(behavior, "WM_DPICHANGED") ||
      !contains(behavior, "WM_MOUSEMOVE") ||
      !contains(behavior, "WM_LBUTTONDOWN") ||
      !contains(behavior, "WM_MOUSEWHEEL") ||
      !contains(source_inventory, "win32_input_coordinates.cpp") ||
      !contains(source_inventory, "win32_window_pointer_events.cpp") ||
      !contains(xmake, "target(\"win32_input_dpi_test\")") ||
      !contains(xmake,
                "target(\"phase_f_win32_input_dpi_structure_test\")")) {
    return 4;
  }
  if (line_count(coordinates_header) > 25 || line_count(coordinates) > 55 ||
      line_count(events) > 95 || line_count(drag) > 115 ||
      line_count(behavior) > 120) {
    return 5;
  }
  constexpr const char* completion =
      "Phase F Step 554 makes Win32 pointer, button, wheel, and native drag "
      "coordinates DPI-aware through focused physical-to-logical conversion "
      "after live scale changes, closing the Win32 input band. Step 555 "
      "Wayland seat capability production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 6;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_554_remaining_gap\": \"Step 555 Wayland")) {
    return 7;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 653 GIF decode boundary production behavior")) {
    return 8;
  }
  return 0;
}
