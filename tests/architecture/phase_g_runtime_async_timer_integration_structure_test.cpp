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
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string platform_header =
      read_source("include/cgpui/platform/platform_application.hpp");
  const std::string platform_defaults = read_source("src/platform/empty.cpp");
  const std::string runtime_internal =
      read_source("src/ui/window_runtime_internal.hpp");
  const std::string timer_integration =
      read_source("src/ui/runtime_timer_integration.cpp");
  const std::string timers = read_source("src/ui/runtime_timers.cpp");
  const std::string scheduling = read_source("src/ui/runtime_scheduling.cpp");
  const std::string win32_header =
      read_source("src/platform/win32/win32_timer_wakeup_internal.hpp");
  const std::string win32_source =
      read_source("src/platform/win32/win32_timer_wakeup.cpp");
  const std::string win32_application =
      read_source("src/platform/win32/win32_application.cpp");
  const std::string wayland_header =
      read_source("src/platform/linux/wayland_timer_wakeup_internal.hpp");
  const std::string wayland_source =
      read_source("src/platform/linux/wayland_timer_wakeup.cpp");
  const std::string wayland_core = read_source(
      "src/platform/linux/wayland_application_core_internal.hpp");
  const std::string wayland_event_loop =
      read_source("src/platform/linux/wayland_event_loop.cpp");
  const std::string behavior =
      read_source("tests/async/runtime_async_timer_integration_test.cpp");
  const std::string win32_behavior =
      read_source("tests/platform/win32_timer_wakeup_test.cpp");
  const std::string wayland_behavior =
      read_source("tests/platform/wayland_timer_wakeup_test.cpp");
  const std::string test_support =
      read_source("tests/ui/window_runtime_platform_test_support.hpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string platform_structure =
      read_source("tests/architecture/platform_source_structure_test.cpp");
  const std::string win32_structure =
      read_source("tests/architecture/win32_window_source_test.cpp");
  const std::string wayland_structure =
      read_source("tests/architecture/wayland_window_source_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_runtime_async_io_hook_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string core_parity = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &platform_header, &platform_defaults, &runtime_internal,
      &timer_integration, &timers, &scheduling, &win32_header, &win32_source,
      &win32_application, &wayland_header, &wayland_source, &wayland_core,
      &wayland_event_loop, &behavior, &win32_behavior, &wayland_behavior,
      &test_support, &ui_structure, &platform_structure, &win32_structure,
      &wayland_structure, &previous, &xmake, &core_parity, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(platform_header, "monotonic_time_ms() const") ||
      !contains(platform_header, "request_wakeup_after(std::uint64_t delay_ms)") ||
      !contains(platform_header, "cancel_wakeup_after()") ||
      !contains(platform_defaults, "std::chrono::steady_clock::now()") ||
      !contains(platform_defaults, "PlatformApplication::request_wakeup_after") ||
      !contains(platform_defaults, "PlatformApplication::cancel_wakeup_after")) {
    return 2;
  }
  if (!contains(runtime_internal, "sync_platform_time()") ||
      !contains(runtime_internal, "schedule_next_timer_wakeup()") ||
      !contains(runtime_internal, "last_platform_time_ms_") ||
      !contains(timer_integration, "application_.monotonic_time_ms()") ||
      !contains(timer_integration, "std::ranges::min_element") ||
      !contains(timer_integration, "application_.request_wakeup_after") ||
      !contains(timer_integration, "application_.cancel_wakeup_after")) return 3;
  if (!contains(timers, "sync_platform_time()") ||
      !contains(timers, "schedule_next_timer_wakeup()") ||
      !contains(scheduling, "sync_platform_time()") ||
      !contains(scheduling, "fire_due_timers()")) return 4;
  if (!contains(win32_header, "class Win32TimerWakeup") ||
      !contains(win32_source, "CreateThreadpoolTimer") ||
      !contains(win32_source, "SetThreadpoolTimer") ||
      !contains(win32_source, "PostThreadMessageW") ||
      !contains(win32_application, "timer_wakeup_.set_thread_id") ||
      !contains(win32_application, "timer_wakeup_.request(delay_ms)")) return 5;
  if (!contains(wayland_header, "class WaylandTimerWakeup") ||
      !contains(wayland_source, "timerfd_create(CLOCK_MONOTONIC") ||
      !contains(wayland_source, "timerfd_settime") ||
      !contains(wayland_core, "WaylandTimerWakeup timer_wakeup_") ||
      !contains(wayland_event_loop, "std::array<pollfd, 3>") ||
      !contains(wayland_event_loop, "timer_wakeup.drain()")) return 6;
  if (!contains(behavior, "std::vector<std::uint64_t>({50, 20, 10})") ||
      !contains(behavior, "schedule_repeating_timer(10") ||
      !contains(behavior, "cancel_timer(long_timer)") ||
      !contains(behavior, "schedule_timer(0") ||
      !contains(win32_behavior, "timer.cancel()") ||
      !contains(wayland_behavior, "timer.cancel()") ||
      !contains(test_support, "advance_monotonic_time")) return 7;
  if (!contains(ui_structure, "src/ui/runtime_timer_integration.cpp") ||
      !contains(platform_structure, "win32_timer_wakeup.cpp") ||
      !contains(platform_structure, "wayland_timer_wakeup.cpp") ||
      !contains(win32_structure, "win32_timer_wakeup.cpp") ||
      !contains(wayland_structure, "wayland_timer_wakeup.cpp") ||
      !contains(previous, "Step 640 async timer integration production behavior is next.") ||
      !contains(xmake, "target(\"phase_g_runtime_async_timer_integration_test\")") ||
      !contains(xmake, "target(\"phase_g_win32_timer_wakeup_test\")") ||
      !contains(xmake, "target(\"phase_g_wayland_timer_wakeup_test\")") ||
      !contains(xmake,
                "target(\"phase_g_runtime_async_timer_integration_structure_test\")")) {
    return 8;
  }
  if (line_count(platform_header) > 70 || line_count(runtime_internal) > 260 ||
      line_count(timer_integration) > 50 || line_count(win32_header) > 50 ||
      line_count(win32_source) > 80 || line_count(win32_application) > 170 ||
      line_count(wayland_header) > 40 || line_count(wayland_source) > 60 ||
      line_count(wayland_core) > 80 || line_count(wayland_event_loop) > 110 ||
      line_count(test_support) > 140 || line_count(behavior) > 130 ||
      line_count(win32_behavior) > 50 || line_count(wayland_behavior) > 40) {
    return 9;
  }

  constexpr const char* completion =
      "Phase G Step 640 integrates runtime timers with platform monotonic "
      "clocks and nearest-deadline delayed wakeups on Win32 and Wayland, "
      "preserving deterministic time advancement, cancellation, repeating "
      "cadence, and zero-delay compatibility. Step 641 cross-thread entity "
      "access production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 10;
  }
  if (contains(core_parity, "timer integration and cross-thread entity access") ||
      contains(core_parity, "complete async timer integration") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 678 Phase G final closeout verification\"")) {
    return 11;
  }
  return 0;
}
