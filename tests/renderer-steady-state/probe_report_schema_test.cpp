#include "probe_report_support.hpp"
#include <limits>
#include <sstream>

namespace {
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
std::string json(const renderer_probe::Options& options, const renderer_probe::Report& report) {
  std::ostringstream out;
  renderer_probe::write_summary(out, options, report);
  return out.str();
}
}
int main() {
  using namespace renderer_probe;
  std::ostringstream escaped;
  quote_json(escaped, std::string{"a\n\"\\\0b", 6});
  if (escaped.str() != R"("a\u000a\"\\\u0000b")") return 1;
  Options options;
  options.samples = 2; options.warmup = 1; options.profile = "dev";
  options.backend = cgpui::RendererBackend::dx12;
  Report report;
  const auto empty = json(options, report);
  if (!has(empty, "\"schema_version\":3") || !has(empty, "\"status\":\"failed\"") ||
      !has(empty, "\"samples_completed\":0") || !has(empty, "\"frame_api\":null") ||
      !has(empty, "\"display_before\":null") || !has(empty, "\"present_mode_runtime_query\":null")) return 2;
  report.status = "valid_baseline"; report.phase = "complete";
  report.measured_creation = RendererCreationSample{1000, CreationOutcome::created};
  report.setup_pixels_valid = report.final_pixels_valid = true;
  report.cold = Sample{.attempts = 1, .frame_wall_ns = 9000000};
  report.warmup_completed = 1;
  report.display_before = DisplayFacts{.device = "display", .hz = 60, .dpi = 96, .valid = true};
  report.display_after = report.display_before;
  report.samples = {Sample{.index = 0, .attempts = 1, .frame_wall_ns = 10},
      Sample{.index = 1, .attempts = 1, .frame_wall_ns = 20}};
  report.actual.adapter_luid = std::numeric_limits<std::uint64_t>::max();
  const auto valid = json(options, report);
  if (!report_valid(options, report) || !has(valid, "\"status\":\"valid_baseline\"") ||
      !has(valid, "\"luid_decimal\":\"18446744073709551615\"") ||
      !has(valid, "\"frame_api\":{\"count\":2,\"min_ns\":10,\"p50_ns\":10,\"p95_ns\":20,\"max_ns\":20,\"mean_ns\":15.000}") ||
      !has(valid, "\"capture_during_measurement\":false") || has(valid, "9000000")) return 3;
  report.samples.pop_back(); // A stale success label cannot certify a partial series.
  if (report_valid(options, report) || !has(json(options, report), "\"status\":\"failed\"")) return 4;
  report.samples.push_back(Sample{.index = 1, .attempts = 1});
  report.final_pixels_valid = false;
  if (report_valid(options, report)) return 5;
  report.final_pixels_valid = true; report.error = "native failure\nkept";
  if (report_valid(options, report) || !has(json(options, report), "native failure\\u000akept")) return 6;
  std::ostringstream raw;
  write_sample_row(raw, Sample{.index = 2, .attempts = 3}, "steady_uncaptured");
  return raw.str() == "steady_uncaptured,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n" ? 0 : 7;
}
