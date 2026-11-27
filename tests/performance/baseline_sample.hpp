#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <span>
#include <string_view>

namespace performance_baseline {

struct WorkloadObservation {
  std::size_t completed_operations = 0;
  bool valid = false;
};

struct Sample {
  std::string_view name;
  std::size_t expected_operations = 0;
  WorkloadObservation observed;
  std::uint64_t total_ns = 0;
};

[[nodiscard]] bool validate_samples(std::span<const Sample> samples, std::ostream& errors);
[[nodiscard]] int write_report(std::ostream& output, std::string_view platform,
                               std::size_t iterations, std::span<const Sample> samples);

} // namespace performance_baseline
