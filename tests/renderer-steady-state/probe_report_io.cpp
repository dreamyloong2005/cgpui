#include "probe_report_support.hpp"
#include <fstream>

namespace renderer_probe {
cgpui::Result<void> write_report(const Options& options, const Report& report) {
  std::ofstream raw(options.output / "samples.csv", std::ios::binary);
  raw << "phase,index,begin_attempts,acquire_wall_ns,begin_api_wall_ns,scheduled_wait_wall_ns,record_api_wall_ns,"
      "present_api_wall_ns,frame_api_wall_ns,diagnostic_pacing_ns,diagnostic_prepare_ns,diagnostic_acquire_ns,"
      "diagnostic_record_ns,diagnostic_submit_ns,diagnostic_present_ns,diagnostic_total_ns,commands,draws,batches,image_bytes,glyph_bytes\n";
  if (report.cold) write_sample_row(raw, *report.cold, "cold_uncaptured");
  for (const auto& s : report.samples) write_sample_row(raw, s, "steady_uncaptured");
  raw.close();
  std::ofstream out(options.output / "report.json", std::ios::binary);
  write_summary(out, options, report);
  out.close();
  if (!raw || !out) return std::unexpected(failure("writing raw samples/report failed"));
  return {};
}
} // namespace renderer_probe
