#include "probe_report_support.hpp"
#include <cstdlib>
#include <ostream>

namespace renderer_probe {
void write_summary(std::ostream& out, const Options& options, const Report& report) {
  out << "{\n\"schema_version\":3,\n\"status\":";
  quote_json(out, report_valid(options, report) ? "valid_baseline" : "failed");
  out << ",\n\"phase\":"; quote_json(out, report.phase);
  out << ",\n\"error\":"; quote_json(out, report.error);
  out << ",\n\"requested_backend\":"; quote_json(out, backend_name(options.backend));
  out << ",\n\"factory_policy\":\"explicit named factory; CGPUI_RENDERER bypassed\",\n\"actual\":";
  write_identity(out, report.actual);
  out << ",\n\"preflight_actual\":";
  if (report.preflight_actual) write_identity(out, *report.preflight_actual); else out << "null";
  out << ",\n\"measured_renderer_creation\":"; write_creation(out, report.measured_creation);
  out << ",\n\"renderer_creation_scope\":\"Measured named-factory wall time only; excludes fixture checks, identity validation, "
      "cold first frame and postflight. Preflight may warm process/driver state. Created means factory success, not whole-run validity.\"";
  out << ",\n\"profile_label_unverified\":"; quote_json(out, options.profile);
  out << ",\n\"source_label_unverified\":"; quote_json(out, options.source_label);
#ifdef NDEBUG
  out << ",\n\"compiled_ndebug\":true";
#else
  out << ",\n\"compiled_ndebug\":false";
#endif
  out << ",\n\"scene\":"; quote_json(out, scene_name(options.scene));
  out << ",\n\"width\":800,\"height\":600,\"scale\":1,\"capture_during_measurement\":false"
      << ",\n\"warmup_requested\":" << options.warmup << ",\"warmup_completed\":" << report.warmup_completed
      << ",\n\"samples_requested\":" << options.samples << ",\"samples_completed\":" << report.samples.size()
      << ",\n\"frame_deadline_ms\":" << options.frame_deadline_ms << ",\"run_deadline_ms\":" << options.run_deadline_ms
      << ",\n\"pending_wait_policy\":\"bounded owner-thread message-aware wait, at most 2ms per retry\""
      << ",\n\"setup_capture_verified\":" << std::boolalpha << report.setup_pixels_valid
      << ",\"post_measurement_capture_verified\":" << report.final_pixels_valid
      << ",\n\"display_before\":"; write_display(out, report.display_before);
  out << ",\n\"display_last_observed\":"; write_display(out, report.display_after);
  out << ",\n\"display_guard\":\"facts before/after each sample, notification epoch for move/size/DPI/display; "
      "unnotified between-sample changes remain unproven; not scanout certification\""
      << ",\"ac_line_status_initial\":" << report.ac_line_status << ",\"remote_session_initial\":" << report.remote_session
      << ",\n\"present_mode_runtime_query\":null,\"driver_version\":null,\"os_build\":null"
      << ",\n\"environment\":{";
  bool first = true;
  for (const auto* name : {"CGPUI_RENDERER", "CGPUI_DX12_ADAPTER", "CGPUI_DX12_DEBUG",
      "CGPUI_DX12_GPU_VALIDATION", "VK_INSTANCE_LAYERS", "VK_LAYER_PATH", "VK_DRIVER_FILES", "VK_ICD_FILENAMES"}) {
    if (!first) out << ','; first = false; quote_json(out, name); out << ':';
    if (const auto* value = std::getenv(name)) quote_json(out, value); else out << "null";
  }
  out << "},\n\"interpretation\":\"CPU wall-clock API envelopes, including waits; not GPU timestamps, scanout, input latency or CPU execution time. "
      "DX12 completes synchronously; Vulkan can retire prior work next frame. Diagnostic stages differ by backend. "
      "Full display checks and message pump before t0/after t3, diagnostic copying, bookkeeping and frame destruction are excluded. "
      "Pending-wait message dispatch/notification bookkeeping stays within acquisition. "
      "No inferred GPU FPS, speedup or performance pass. Pixel validation is separate from measured frames.\","
      "\n\"percentiles\":\"nearest-rank, no discarded outliers; steady samples only, failed partial rows remain failed\",\n\"statistics\":{";
  write_statistic(out, report.samples, &Sample::acquire_wall_ns, "acquire"); out << ',';
  write_statistic(out, report.samples, &Sample::begin_api_wall_ns, "begin_api"); out << ',';
  write_statistic(out, report.samples, &Sample::record_wall_ns, "record_api"); out << ',';
  write_statistic(out, report.samples, &Sample::present_wall_ns, "present_api"); out << ',';
  write_statistic(out, report.samples, &Sample::frame_wall_ns, "frame_api"); out << "}\n}\n";
}
} // namespace renderer_probe
