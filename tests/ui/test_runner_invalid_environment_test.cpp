// run_test with an invalid ITERATIONS variable reports invalid_configuration
// and runs nothing (the xmake test case "invalid_environment" of
// phase_g_test_runner_test).
#include "cgpui/ui/test_runner.hpp"
#include "test_environment.hpp"

#include <string_view>

namespace {

void never_run(cgpui::TestRunSeed) {}

} // namespace

int main() {
  if (!set_test_environment("ITERATIONS", "invalid")) {
    return 90;
  }
  const cgpui::TestRunSummary invalid_environment = cgpui::run_test(
      "invalid_environment", &never_run, cgpui::TestRunOptions{});
  return invalid_environment.status ==
            cgpui::TestRunStatus::invalid_configuration &&
        invalid_environment.attempt_count == 0 &&
        invalid_environment.failure.has_value() &&
        invalid_environment.failure->message ==
            std::string_view{"ITERATIONS must be an unsigned integer"}
        ? 0
        : 4;
}
