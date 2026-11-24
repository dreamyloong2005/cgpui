#include "probe_report_support.hpp"

int main() {
  using namespace renderer_probe;
  Options options;
  options.samples = 1; options.warmup = 1;
  Report report;
  report.status = "valid_baseline"; report.phase = "complete";
  report.measured_creation = RendererCreationSample{1000, CreationOutcome::created};
  report.setup_pixels_valid = report.final_pixels_valid = true;
  report.cold = Sample{}; report.warmup_completed = 1;
  report.samples = {Sample{.index = 0, .attempts = 1}};
  report.display_before = DisplayFacts{.device = "display-A", .width = 1920, .height = 1080,
      .hz = 60, .dpi = 96, .event_epoch = 4, .valid = true};
  report.display_after = report.display_before;
  if (!report_valid(options, report)) return 1;
  report.display_after->device = "display-B";
  if (report_valid(options, report)) return 2;
  report.display_after = report.display_before; report.display_after->hz = 144;
  if (report_valid(options, report)) return 3;
  report.display_after = report.display_before; report.display_after->dpi = 144;
  if (report_valid(options, report)) return 4;
  report.display_after = report.display_before; report.display_after->width = 2560;
  if (report_valid(options, report)) return 5;
  // A notified switch and return must stay invalid even though endpoint facts match.
  report.display_after = report.display_before; report.display_after->event_epoch += 2;
  if (report_valid(options, report)) return 6;
  report.display_after = report.display_before; report.display_after->window_left = 1;
  if (report_valid(options, report)) return 7;
  report.display_after = report.display_before; report.display_after->valid = false;
  if (report_valid(options, report)) return 8;
  report.display_after.reset();
  return report_valid(options, report) ? 9 : 0;
}
