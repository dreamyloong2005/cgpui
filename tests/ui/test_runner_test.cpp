#include "cgpui/ui/test_runner.hpp"

#include <array>
#include <cstdlib>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

std::array<std::size_t, 10> retry_attempts{};
std::vector<std::uint64_t> retry_seeds;
std::vector<std::uint64_t> environment_seeds;
std::optional<cgpui::TestRunFailure> reported_failure;

void retry_once_per_seed(
    cgpui::TestApp& first,
    cgpui::TestApp& second,
    cgpui::TestRunSeed seed) {
  if (first.read_from_clipboard().has_value() ||
      second.read_from_clipboard().has_value() ||
      !first.write_to_clipboard("runner isolation") ||
      second.read_from_clipboard().has_value()) {
    throw std::runtime_error("TestApp instances were not isolated");
  }
  retry_seeds.push_back(seed.value);
  if (retry_attempts.at(seed.value)++ == 0) {
    throw std::runtime_error("retry this seed once");
  }
}

void record_environment_seed(cgpui::TestRunSeed seed) {
  environment_seeds.push_back(seed.value);
}

void always_fail(cgpui::TestApp&, cgpui::TestRunSeed seed) {
  if (seed.value != 42) throw std::runtime_error("unexpected failure seed");
  throw std::runtime_error("final failure");
}

void record_failure(const cgpui::TestRunFailure& failure) {
  reported_failure = failure;
}

} // namespace

int main() {
  if (std::getenv("CGPUI_EXPECT_INVALID_ENV") != nullptr) {
    const cgpui::TestRunSummary invalid_environment = cgpui::run_test(
        "invalid_environment",
        &record_environment_seed,
        cgpui::TestRunOptions{});
    return invalid_environment.status ==
            cgpui::TestRunStatus::invalid_configuration &&
        invalid_environment.attempt_count == 0 &&
        invalid_environment.failure.has_value() &&
        invalid_environment.failure->message ==
            std::string_view{"ITERATIONS must be an unsigned integer"}
        ? 0
        : 4;
  }

  const cgpui::TestRunSummary retry_summary = cgpui::run_test(
      "retry_once_per_seed",
      &retry_once_per_seed,
      cgpui::TestRunOptions{
          .iterations = 2,
          .seeds = {9},
          .max_retries = 1,
          .use_environment = false,
      });
  if (!retry_summary.passed() || retry_summary.run_count != 3 ||
      retry_summary.attempt_count != 6 || retry_summary.retry_count != 3 ||
      retry_summary.failure.has_value() ||
      retry_seeds != std::vector<std::uint64_t>{0, 0, 1, 1, 9, 9}) {
    return 1;
  }

  const cgpui::TestRunSummary environment_summary = cgpui::run_test(
      "record_environment_seed",
      &record_environment_seed,
      cgpui::TestRunOptions{.iterations = 1, .seeds = {99}});
  if (!environment_summary.passed() || environment_summary.run_count != 2 ||
      environment_seeds != std::vector<std::uint64_t>{50, 51}) {
    return 2;
  }

  const cgpui::TestRunSummary failure_summary = cgpui::run_test(
      "always_fail",
      &always_fail,
      cgpui::TestRunOptions{
          .iterations = 1,
          .seeds = {42},
          .max_retries = 1,
          .use_environment = false,
          .on_failure = &record_failure,
      });
  if (failure_summary.passed() || failure_summary.run_count != 1 ||
      failure_summary.attempt_count != 2 || failure_summary.retry_count != 1 ||
      !failure_summary.failure.has_value() ||
      failure_summary.failure->seed != 42 ||
      failure_summary.failure->attempt != 2 ||
      failure_summary.failure->message != std::string_view{"final failure"} ||
      reported_failure != failure_summary.failure) {
    return 3;
  }
  const cgpui::TestRunSummary null_summary = cgpui::run_test(
      "null",
      static_cast<void (*)(cgpui::TestApp&)>(nullptr),
      cgpui::TestRunOptions{.use_environment = false});
  if (null_summary.status != cgpui::TestRunStatus::invalid_configuration ||
      !null_summary.failure.has_value() ||
      null_summary.failure->message != std::string_view{"test function is null"}) {
    return 5;
  }
  const cgpui::TestRunSummary retry_overflow = cgpui::run_test(
      "retry_overflow",
      &record_environment_seed,
      cgpui::TestRunOptions{
          .max_retries = std::numeric_limits<std::size_t>::max(),
          .use_environment = false,
      });
  if (retry_overflow.status != cgpui::TestRunStatus::invalid_configuration ||
      retry_overflow.attempt_count != 0 || !retry_overflow.failure.has_value() ||
      retry_overflow.failure->message !=
          std::string_view{"test retry count is too large"}) {
    return 6;
  }
  return 0;
}
