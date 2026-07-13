#include "cgpui/ui/test_runner.hpp"

#include <charconv>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace cgpui::detail {
namespace {

struct EnvironmentNumber {
  bool present = false;
  bool valid = true;
  std::uint64_t value = 0;
};

EnvironmentNumber read_environment_number(const char* name) {
  const char* text = std::getenv(name);
  if (text == nullptr) return {};
  EnvironmentNumber result{.present = true};
  const std::string_view value{text};
  const auto [end, error] =
      std::from_chars(value.data(), value.data() + value.size(), result.value);
  result.valid = error == std::errc{} && end == value.data() + value.size();
  return result;
}

TestRunSummary invalid_configuration(std::string message) {
  return TestRunSummary{
      .status = TestRunStatus::invalid_configuration,
      .failure = TestRunFailure{.message = std::move(message)},
  };
}

std::optional<TestRunSummary> resolve_test_seeds(
    const TestRunOptions& options,
    std::vector<std::uint64_t>& seeds) {
  std::size_t iterations = options.iterations;
  EnvironmentNumber environment_seed;
  if (options.use_environment) {
    const EnvironmentNumber environment_iterations =
        read_environment_number("ITERATIONS");
    environment_seed = read_environment_number("SEED");
    if (!environment_iterations.valid) {
      return invalid_configuration("ITERATIONS must be an unsigned integer");
    }
    if (!environment_seed.valid) {
      return invalid_configuration("SEED must be an unsigned integer");
    }
    if (environment_iterations.present) {
      if (environment_iterations.value == 0 ||
          environment_iterations.value >
              std::numeric_limits<std::size_t>::max()) {
        return invalid_configuration("ITERATIONS must be greater than zero");
      }
      iterations = static_cast<std::size_t>(environment_iterations.value);
    }
  }
  if (iterations == 0) {
    return invalid_configuration("test iterations must be greater than zero");
  }

  const bool seeded_from_environment = environment_seed.present;
  const std::uint64_t first_seed =
      seeded_from_environment ? environment_seed.value : 0;
  if (first_seed > std::numeric_limits<std::uint64_t>::max() -
          static_cast<std::uint64_t>(iterations - 1)) {
    return invalid_configuration("test seed range overflows uint64");
  }
  if (iterations != 1 ||
      (!seeded_from_environment && options.seeds.empty())) {
    seeds.reserve(iterations +
        (seeded_from_environment ? 0 : options.seeds.size()));
    for (std::size_t index = 0; index < iterations; ++index) {
      seeds.push_back(first_seed + static_cast<std::uint64_t>(index));
    }
  } else if (seeded_from_environment) {
    seeds.push_back(first_seed);
  }
  if (!seeded_from_environment) {
    seeds.insert(seeds.end(), options.seeds.begin(), options.seeds.end());
  }
  return std::nullopt;
}

std::string current_exception_message() {
  try {
    throw;
  } catch (const std::exception& error) {
    return error.what();
  } catch (...) {
    return "unknown test exception";
  }
}

} // namespace

TestRunSummary invalid_test_run(std::string_view, std::string message) {
  return invalid_configuration(std::move(message));
}

TestRunSummary run_test_erased(
    std::string_view,
    TestRunOptions options,
    void* context,
    TestRunInvocation invocation) {
  if (context == nullptr || invocation == nullptr) {
    return invalid_configuration("test invocation is null");
  }
  if (options.max_retries == std::numeric_limits<std::size_t>::max()) {
    return invalid_configuration("test retry count is too large");
  }
  std::vector<std::uint64_t> seeds;
  if (auto invalid = resolve_test_seeds(options, seeds)) return *invalid;

  TestRunSummary summary;
  for (const std::uint64_t seed : seeds) {
    summary.run_count += 1;
    for (std::size_t attempt = 1;; ++attempt) {
      summary.attempt_count += 1;
      try {
        invocation(context, seed);
        break;
      } catch (...) {
        const std::string message = current_exception_message();
        if (attempt <= options.max_retries) {
          summary.retry_count += 1;
          continue;
        }
        summary.status = TestRunStatus::failed;
        summary.failure = TestRunFailure{
            .seed = seed,
            .attempt = attempt,
            .message = message,
        };
        if (options.on_failure != nullptr) {
          options.on_failure(*summary.failure);
        }
        return summary;
      }
    }
  }
  return summary;
}

int test_main_exit_code(
    std::string_view name,
    const TestRunSummary& summary) {
  if (summary.passed()) return 0;
  std::cerr << name << ": ";
  if (summary.status == TestRunStatus::invalid_configuration) {
    std::cerr << "invalid test configuration";
  } else {
    std::cerr << "failed for seed " << summary.failure->seed
              << " on attempt " << summary.failure->attempt;
  }
  if (summary.failure && !summary.failure->message.empty()) {
    std::cerr << ": " << summary.failure->message;
  }
  std::cerr << '\n';
  return 1;
}

} // namespace cgpui::detail
