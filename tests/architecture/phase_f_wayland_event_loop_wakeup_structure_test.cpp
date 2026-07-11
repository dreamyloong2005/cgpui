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
  const std::string event_loop = read_source(
      "src/platform/linux/wayland_event_loop.cpp");
  const std::string lifecycle = read_source(
      "src/platform/linux/wayland_application_lifecycle.cpp");
  const std::string core = read_source(
      "src/platform/linux/wayland_application_core_internal.hpp");
  const std::string input = read_source(
      "src/platform/linux/wayland_input_internal.hpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_event_loop_wakeup_test.cpp");
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
      &event_loop, &lifecycle, &core, &input, &behavior, &xmake, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) if (source->empty()) return 1;
  if (!contains(event_loop, "wl_display_prepare_read(display)") ||
      !contains(event_loop, "wl_display_read_events(display)") ||
      !contains(event_loop, "wl_display_cancel_read(display)") ||
      !contains(event_loop, "flush_pending ? POLLOUT : 0")) return 2;
  if (!contains(event_loop, "errno == EINTR") ||
      !contains(event_loop, "drain_wakeup_pipe(wakeup_read_fd)") ||
      !contains(event_loop, "if (running.load()) dispatch_wakeup(windows)") ||
      !contains(event_loop, "wayland_request_wakeup(")) return 3;
  if (!contains(core, "std::atomic_bool running_{true}") ||
      !contains(input, "std::atomic_bool& running") ||
      !contains(lifecycle, "running_.store(false)")) return 4;
  if (!contains(behavior, "request < 32") ||
      !contains(behavior, "wakeup_count.load() != 1") ||
      !contains(behavior, "wakeup_count.load() != 2") ||
      !contains(behavior, "(*app)->quit()") ||
      !contains(xmake, "target(\"wayland_event_loop_wakeup_test\")") ||
      !contains(xmake,
                "target(\"phase_f_wayland_event_loop_wakeup_structure_test\")")) {
    return 5;
  }
  if (line_count(event_loop) > 110 || line_count(lifecycle) > 25 ||
      line_count(core) > 80 || line_count(behavior) > 110) return 6;
  constexpr const char* completion =
      "Phase F Step 562 makes Wayland wakeups thread-safe with atomic run "
      "state, prepare-read polling, EINTR-safe pipe draining, burst "
      "coalescing, and quit-only wake suppression. Step 563 Win32 Unicode "
      "clipboard production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) return 7;
  }
  if (!contains(ledger_json,
                "\"phase_f_step_562_remaining_gap\": \"Step 563 Win32")) {
    return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 581 native menu")) {
    return 9;
  }
  return 0;
}
