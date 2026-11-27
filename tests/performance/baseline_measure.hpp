#pragma once

#include "baseline_sample.hpp"

#include <chrono>

namespace performance_baseline {

// Workloads record public observations while running. Final validation and
// report publication happen after the measured interval, never through a sink
// shared by unrelated workloads.
template <typename Work, typename Observe>
Sample measure(std::string_view name, std::size_t expected_operations,
               Work&& work, Observe&& observe) {
  using Clock = std::chrono::steady_clock;
  const auto start = Clock::now();
  work();
  const auto end = Clock::now();
  return Sample{
      .name = name,
      .expected_operations = expected_operations,
      .observed = observe(),
      .total_ns = static_cast<std::uint64_t>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()),
  };
}

} // namespace performance_baseline
