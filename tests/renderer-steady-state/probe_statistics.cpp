#include "probe_report_support.hpp"
#include <algorithm>
#include <iomanip>
#include <ostream>

namespace renderer_probe {
std::optional<Statistics> statistics(std::span<const std::uint64_t> values) {
  if (values.empty()) return {};
  std::vector<std::uint64_t> sorted(values.begin(), values.end());
  std::sort(sorted.begin(), sorted.end());
  long double total = 0;
  for (const auto value : values) total += value;
  // ceil(0.95*n)-1 without multiplying n or requiring floating rounding.
  const auto p95_index = sorted.size() - sorted.size() / 20 - 1;
  return Statistics{values.size(), sorted.front(), sorted[(sorted.size() - 1) / 2],
      sorted[p95_index], sorted.back(), total / values.size()};
}
bool report_valid(const Options& options, const Report& report) {
  if (report.status != "valid_baseline" || report.phase != "complete" || !report.error.empty() ||
      !report.setup_pixels_valid || !report.final_pixels_valid || !report.cold ||
      !report.measured_creation || report.measured_creation->outcome != CreationOutcome::created ||
      report.warmup_completed != options.warmup || report.samples.size() != options.samples ||
      options.samples == 0 || !report.display_before || !report.display_before->valid ||
      report.display_before != report.display_after) return false;
  for (std::size_t index = 0; index != report.samples.size(); ++index)
    if (report.samples[index].index != index || report.samples[index].attempts == 0) return false;
  return true;
}
void write_statistic(std::ostream& out, std::span<const Sample> samples,
    std::uint64_t Sample::*field, std::string_view name) {
  quote_json(out, name); out << ':';
  std::vector<std::uint64_t> values; values.reserve(samples.size());
  for (const auto& sample : samples) values.push_back(sample.*field);
  const auto summary = statistics(values);
  if (!summary) { out << "null"; return; }
  out << "{\"count\":" << summary->count << ",\"min_ns\":" << summary->minimum
      << ",\"p50_ns\":" << summary->p50 << ",\"p95_ns\":" << summary->p95
      << ",\"max_ns\":" << summary->maximum << ",\"mean_ns\":"
      << std::fixed << std::setprecision(3) << summary->mean << '}';
}
} // namespace renderer_probe
