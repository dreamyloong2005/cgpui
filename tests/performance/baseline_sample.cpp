#include "baseline_sample.hpp"

#include <array>
#include <ostream>

namespace performance_baseline {

bool validate_samples(std::span<const Sample> samples, std::ostream& errors) {
  constexpr std::array names{"startup", "first_frame", "resize", "text_layout",
      "list_scrolling", "image_loading", "async_wakeup", "frame_pacing"};
  if (samples.size() != names.size()) {
    errors << "performance baseline: incomplete workload inventory\n";
    return false;
  }
  bool valid = true;
  for (std::size_t index = 0; index < names.size(); ++index) {
    const auto& sample = samples[index];
    if (sample.name != names[index] || sample.expected_operations == 0 ||
        !sample.observed.valid ||
        sample.observed.completed_operations != sample.expected_operations) {
      errors << "performance baseline: " << names[index] << " did not complete its workload ("
             << sample.observed.completed_operations << '/' << sample.expected_operations
             << ", observations=" << (sample.observed.valid ? "valid" : "invalid") << ")\n";
      valid = false;
    }
  }
  return valid;
}

int write_report(std::ostream& output, std::string_view platform,
                 std::size_t iterations, std::span<const Sample> samples) {
  output << "{\n  \"schema_version\": 1,\n  \"platform\": \"" << platform
         << "\",\n  \"iterations\": " << iterations << ",\n  \"metrics\": [\n";
  for (std::size_t index = 0; index < samples.size(); ++index) {
    const auto& sample = samples[index];
    const auto count = sample.observed.completed_operations;
    output << "    {\"name\": \"" << sample.name
           << "\", \"unit\": \"ns_per_operation\", \"operation_count\": " << count
           << ", \"total_ns\": " << sample.total_ns
           << ", \"mean_ns\": " << (count == 0 ? 0 : sample.total_ns / count) << "}"
           << (index + 1 == samples.size() ? "\n" : ",\n");
  }
  output << "  ]\n}\n";
  return output ? 0 : 5;
}

} // namespace performance_baseline
