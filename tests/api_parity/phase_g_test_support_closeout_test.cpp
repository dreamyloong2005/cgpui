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

} // namespace

int main() {
  const std::string setup = read_source(
      "tests/architecture/phase_g_test_app_window_setup_structure_test.cpp");
  const std::string input = read_source(
      "tests/architecture/phase_g_test_app_simulated_input_structure_test.cpp");
  const std::string timers = read_source(
      "tests/architecture/phase_g_test_app_timer_control_structure_test.cpp");
  const std::string async = read_source(
      "tests/architecture/phase_g_test_app_async_control_structure_test.cpp");
  const std::string rendering = read_source(
      "tests/architecture/phase_g_test_app_rendering_control_structure_test.cpp");
  const std::string services = read_source(
      "tests/architecture/phase_g_test_app_platform_services_structure_test.cpp");
  const std::string runner = read_source(
      "tests/architecture/phase_g_test_runner_structure_test.cpp");
  const std::string app_cleanliness = read_source(
      "tests/header_cleanliness/test_app_header_cleanliness.cpp");
  const std::string runner_cleanliness = read_source(
      "tests/header_cleanliness/test_runner_header_cleanliness.cpp");
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
      &setup, &input, &timers, &async, &rendering, &services, &runner,
      &app_cleanliness, &runner_cleanliness, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(setup, "class TestAppWindow") ||
      !contains(input, "TestAppWindow::simulate_keystrokes(") ||
      !contains(timers, "advance_time(") ||
      !contains(async, "complete_task(") ||
      !contains(rendering, "try_draw_frame(") ||
      !contains(services, "TestPlatformServiceSnapshot") ||
      !contains(runner, "#define CGPUI_TEST_WITH_OPTIONS")) return 2;

  const char* targets[]{
      "phase_g_test_app_window_fixture_test",
      "phase_g_test_app_simulated_input_test",
      "phase_g_test_app_timer_control_test",
      "phase_g_test_app_async_control_test",
      "phase_g_test_app_rendering_control_test",
      "phase_g_test_app_platform_services_test",
      "phase_g_test_runner_test",
      "phase_g_test_runner_macro_test",
      "phase_g_test_app_window_setup_structure_test",
      "phase_g_test_app_simulated_input_structure_test",
      "phase_g_test_app_timer_control_structure_test",
      "phase_g_test_app_async_control_structure_test",
      "phase_g_test_app_rendering_control_structure_test",
      "phase_g_test_app_platform_services_structure_test",
      "phase_g_test_runner_structure_test",
      "test_app_header_cleanliness",
      "test_runner_header_cleanliness",
      "phase_g_test_support_closeout_test"};
  for (const char* target : targets) {
    if (!contains(xmake, (std::string{"target(\""} + target + "\")").c_str())) {
      return 3;
    }
  }
  if (!contains(app_cleanliness, "#include \"cgpui/ui/test_app.hpp\"") ||
      !contains(runner_cleanliness,
                "#include \"cgpui/ui/test_runner.hpp\"") ||
      !contains(setup, "src/ui/test_app.cpp") ||
      !contains(input, "src/ui/test_app_input.cpp") ||
      !contains(timers, "src/ui/test_app_timer.cpp") ||
      !contains(async, "src/ui/test_app_async.cpp") ||
      !contains(rendering, "src/ui/test_app_rendering.cpp") ||
      !contains(services, "src/ui/test_app_platform_services.cpp") ||
      !contains(services, "src/ui/test_app_services.cpp") ||
      !contains(runner, "src/ui/test_runner.cpp") ||
      !contains(roadmap, "- [x] Steps 659-666: Add GPUI-style test")) {
    return 4;
  }

  constexpr const char* completion =
      "Phase G Step 666 audits and closes the GPUI-style test support band, "
      "freezing standalone app/window setup, simulated input, deterministic "
      "timer and async control, rendering control, platform service fakes, "
      "test runner ergonomics, public header cleanliness, and modular source "
      "evidence. Step 667 Windows debug build and packaging coverage is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 5;
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 676 candidate-ledger closeout audit\"")) {
    return 6;
  }
  return 0;
}
