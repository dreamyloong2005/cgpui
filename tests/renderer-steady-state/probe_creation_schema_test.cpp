#include "probe_report_support.hpp"
#include <array>
#include <sstream>

namespace {
bool has(const std::string& value, const char* token) { return value.find(token) != std::string::npos; }
std::string summary(const renderer_probe::Options& options, const renderer_probe::Report& report) {
  std::ostringstream out;
  renderer_probe::write_summary(out, options, report);
  return out.str();
}
}
int main() {
  using namespace renderer_probe;
  Options options;
  options.samples = 2; options.warmup = 1;
  Report report;
  const auto absent = summary(options, report);
  if (!has(absent, "\"measured_renderer_creation\":null") || report_valid(options, report)) return 1;
  report.status = "valid_baseline"; report.phase = "complete";
  report.setup_pixels_valid = report.final_pixels_valid = true;
  report.warmup_completed = 1; report.cold = Sample{.attempts = 1, .frame_wall_ns = 4000000};
  report.samples = {Sample{.index = 0, .attempts = 1, .frame_wall_ns = 10},
                    Sample{.index = 1, .attempts = 1, .frame_wall_ns = 20}};
  report.display_before = DisplayFacts{.device = "display", .width = 800, .height = 600,
      .hz = 60, .dpi = 96, .valid = true};
  report.display_after = report.display_before;
  if (report_valid(options, report)) return 2; // No factory observation cannot become success.
  constexpr std::array outcomes{CreationOutcome::factory_error, CreationOutcome::null_renderer,
      CreationOutcome::exception};
  constexpr std::array names{"factory_error", "null_renderer", "exception"};
  for (unsigned i = 0; i != outcomes.size(); ++i) {
    report.measured_creation = RendererCreationSample{123, outcomes[i]};
    const auto failed = summary(options, report);
    const auto observation = std::string("\"measured_renderer_creation\":{\"wall_ns\":123,\"outcome\":\"") + names[i] + "\"}";
    if (report_valid(options, report) || !has(failed, "\"status\":\"failed\"") || !has(failed, observation.c_str())) return 3;
  }
  report.measured_creation = RendererCreationSample{9000000, CreationOutcome::created};
  const auto valid = summary(options, report);
  if (!report_valid(options, report) || !has(valid, "\"schema_version\":3") ||
      !has(valid, "\"measured_renderer_creation\":{\"wall_ns\":9000000,\"outcome\":\"created\"}") ||
      !has(valid, "\"frame_api\":{\"count\":2,\"min_ns\":10,\"p50_ns\":10,\"p95_ns\":20,\"max_ns\":20,\"mean_ns\":15.000}") ||
      has(valid, "4000000")) return 4; // Neither creation nor cold enters steady statistics.
  report.error = "actual adapter identity check failed";
  const auto identity_failed = summary(options, report);
  if (report_valid(options, report) || !has(identity_failed, "\"status\":\"failed\"") ||
      !has(identity_failed, "\"outcome\":\"created\"")) return 5; // Factory return is not whole-run success.
  report.error.clear(); report.final_pixels_valid = false;
  if (report_valid(options, report)) return 6;
  std::ostringstream raw;
  write_sample_row(raw, report.samples.front(), "steady_uncaptured");
  return !has(raw.str(), "9000000") && !has(raw.str(), "4000000") ? 0 : 7;
}
