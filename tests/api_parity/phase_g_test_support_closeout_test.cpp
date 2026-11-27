#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string manifests = read_source("mcpp.toml") +
      read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml") +
      read_source("tests/header_cleanliness/mcpp.toml") +
      read_source("tests/ui/mcpp.toml");
  const std::string* required[]{
      &setup,
      &input,
      &timers,
      &async,
      &rendering,
      &services,
      &runner,
      &app_cleanliness,
      &runner_cleanliness,
      &manifests};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(setup, "class TestAppWindow") ||
      !contains(input, "TestAppWindow::simulate_keystrokes(") ||
      !contains(timers, "advance_time(") ||
      !contains(async, "complete_task(") ||
      !contains(rendering, "try_draw_frame(") ||
      !contains(services, "TestPlatformServiceSnapshot") ||
      !contains(runner, "#define CGPUI_TEST_WITH_OPTIONS")) return 2;

  const char* targets[]{
      "test_app_window_setup_test.cpp",
      "test_app_simulated_input_test.cpp",
      "test_app_timer_control_test.cpp",
      "test_app_async_control_test.cpp",
      "test_app_rendering_control_test.cpp",
      "test_app_platform_services_test.cpp",
      "test_runner_test.cpp",
      "test_runner_macro_test.cpp",
      "phase_g_test_app_window_setup_structure_test.cpp",
      "phase_g_test_app_simulated_input_structure_test.cpp",
      "phase_g_test_app_timer_control_structure_test.cpp",
      "phase_g_test_app_async_control_structure_test.cpp",
      "phase_g_test_app_rendering_control_structure_test.cpp",
      "phase_g_test_app_platform_services_structure_test.cpp",
      "phase_g_test_runner_structure_test.cpp",
      "test_app_header_cleanliness.cpp",
      "test_runner_header_cleanliness.cpp",
      "phase_g_test_support_closeout_test.cpp"};
  for (const char* target : targets) {
    if (!contains(manifests, (std::string{"\""} + target + "\"").c_str())) {
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
      !contains(runner, "src/ui/test_runner.cpp")) {
    return 4;
  }

  return 0;
}
