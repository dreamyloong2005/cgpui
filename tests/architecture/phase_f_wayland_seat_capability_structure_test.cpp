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
  const std::string seat =
      read_source("src/platform/linux/wayland_application_seat.cpp");
  const std::string application =
      read_source("src/platform/linux/wayland_application.cpp");
  const std::string behavior =
      read_source("tests/platform/wayland_seat_capability_test.cpp");
  const std::string compositor_header =
      read_source("tests/platform/wayland_test_compositor.hpp");
  const std::string compositor =
      read_source("tests/platform/wayland_test_compositor.cpp");
  const std::string source_inventory =
      read_source("tests/architecture/wayland_window_source_test.cpp");
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
      &seat, &application, &behavior, &compositor_header, &compositor,
      &source_inventory, &xmake, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(seat, "WL_POINTER_RELEASE_SINCE_VERSION") ||
      !contains(seat, "wl_pointer_release(") ||
      !contains(seat, "WL_KEYBOARD_RELEASE_SINCE_VERSION") ||
      !contains(seat, "wl_keyboard_release(") ||
      !contains(seat, "wayland_window_focus_changed(") ||
      !contains(seat, "pointer_enter_serial_ = 0") ||
      !contains(seat, "wayland_text_input_reset(")) {
    return 2;
  }
  if (contains(application, "WaylandApplication::handle_seat_capabilities(") ||
      !contains(source_inventory, "wayland_application_seat.cpp") ||
      !contains(source_inventory, "wl_pointer_release(") ||
      !contains(source_inventory, "wl_keyboard_release(")) {
    return 3;
  }
  if (!contains(behavior, "WL_SEAT_CAPABILITY_KEYBOARD") ||
      !contains(behavior, "WL_SEAT_CAPABILITY_POINTER") ||
      !contains(behavior, "wait_for_pointer_bound(false)") ||
      !contains(behavior, "wait_for_keyboard_bound(false)") ||
      !contains(compositor_header, "request_seat_capabilities(") ||
      !contains(compositor, "dispatch_pending_seat_capabilities(") ||
      !contains(compositor, ".release = destroy_resource") ||
      !contains(xmake, "target(\"wayland_seat_capability_test\")") ||
      !contains(xmake,
                "target(\"phase_f_wayland_seat_capability_structure_test\")")) {
    return 4;
  }
  if (line_count(seat) > 120 || line_count(behavior) > 150) {
    return 5;
  }
  constexpr const char* completion =
      "Phase F Step 555 adds live Wayland seat capability transitions with "
      "version-aware pointer and keyboard release, removal-time focus loss, "
      "stale input-state cleanup, and proxy reacquisition. Step 556 Wayland "
      "keyboard layout and modifier production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 6;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_555_remaining_gap\": \"Step 556 Wayland")) {
    return 7;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 626 Win32 UIA production closeout audit")) {
    return 8;
  }
  return 0;
}
