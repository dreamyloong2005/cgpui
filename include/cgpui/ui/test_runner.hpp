#pragma once

#include "cgpui/ui/test_app.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace cgpui {

struct TestRunSeed {
  std::uint64_t value = 0;
};

enum class TestRunStatus {
  passed,
  failed,
  invalid_configuration,
};

struct TestRunFailure {
  std::uint64_t seed = 0;
  std::size_t attempt = 0;
  std::string message;

  friend bool operator==(const TestRunFailure&, const TestRunFailure&) = default;
};

using TestRunFailureCallback = void (*)(const TestRunFailure& failure);

struct TestRunOptions {
  std::size_t iterations = 1;
  std::vector<std::uint64_t> seeds;
  std::size_t max_retries = 0;
  bool use_environment = true;
  TestRunFailureCallback on_failure = nullptr;
};

struct TestRunSummary {
  TestRunStatus status = TestRunStatus::passed;
  std::size_t run_count = 0;
  std::size_t attempt_count = 0;
  std::size_t retry_count = 0;
  std::optional<TestRunFailure> failure;

  [[nodiscard]] constexpr bool passed() const {
    return status == TestRunStatus::passed;
  }
};

namespace detail {

using TestRunInvocation = void (*)(void* context, std::uint64_t seed);

[[nodiscard]] TestRunSummary run_test_erased(
    std::string_view name,
    TestRunOptions options,
    void* context,
    TestRunInvocation invocation);
[[nodiscard]] TestRunSummary invalid_test_run(
    std::string_view name,
    std::string message);
[[nodiscard]] int test_main_exit_code(
    std::string_view name,
    const TestRunSummary& summary);

template <typename Argument>
inline constexpr bool is_test_app_argument =
    std::same_as<Argument, TestApp&>;

template <typename Argument>
inline constexpr bool is_test_seed_argument =
    std::same_as<std::remove_cvref_t<Argument>, TestRunSeed>;

template <typename Argument>
concept TestRunArgument =
    is_test_app_argument<Argument> || is_test_seed_argument<Argument>;

template <TestRunArgument Argument>
using TestRunArgumentStorage = std::conditional_t<
    is_test_app_argument<Argument>,
    TestApp,
    TestRunSeed>;

template <TestRunArgument Argument>
TestRunArgumentStorage<Argument> make_test_run_argument(std::uint64_t seed) {
  if constexpr (is_test_app_argument<Argument>) {
    return TestApp{};
  } else {
    return TestRunSeed{.value = seed};
  }
}

template <TestRunArgument Argument>
decltype(auto) test_run_argument(TestRunArgumentStorage<Argument>& value) {
  return static_cast<Argument>(value);
}

template <typename Value>
void drain_test_app(Value& value) {
  if constexpr (std::same_as<Value, TestApp>) value.run_until_parked();
}

} // namespace detail

template <typename... Arguments>
  requires (detail::TestRunArgument<Arguments> && ...)
[[nodiscard]] TestRunSummary run_test(
    std::string_view name,
    void (*test_function)(Arguments...),
    TestRunOptions options = {}) {
  if (test_function == nullptr) {
    return detail::invalid_test_run(name, "test function is null");
  }
  struct TestFunctionState {
    void (*function)(Arguments...);
  } state{.function = test_function};
  return detail::run_test_erased(
      name,
      std::move(options),
      &state,
      [](void* context, std::uint64_t seed) {
        auto* function_state = static_cast<TestFunctionState*>(context);
        auto arguments = std::tuple<detail::TestRunArgumentStorage<Arguments>...>{
            detail::make_test_run_argument<Arguments>(seed)...};
        std::apply(
            [function_state](auto&... stored) {
              function_state->function(
                  detail::test_run_argument<Arguments>(stored)...);
            },
            arguments);
        std::apply(
            [](auto&... stored) {
              (detail::drain_test_app(stored), ...);
            },
            arguments);
      });
}

template <typename... Arguments>
  requires (detail::TestRunArgument<Arguments> && ...)
[[nodiscard]] int test_main(
    std::string_view name,
    void (*test_function)(Arguments...),
    TestRunOptions options = {}) {
  return detail::test_main_exit_code(
      name,
      run_test(name, test_function, std::move(options)));
}

} // namespace cgpui

#define CGPUI_TEST_WITH_OPTIONS(name, options, ...)                       \
  static void name(__VA_ARGS__);                                         \
  int main() {                                                           \
    return ::cgpui::test_main(#name, &name, (options));                  \
  }                                                                      \
  static void name(__VA_ARGS__)

#define CGPUI_TEST(name, ...)                                            \
  CGPUI_TEST_WITH_OPTIONS(name, ::cgpui::TestRunOptions{}, __VA_ARGS__)
