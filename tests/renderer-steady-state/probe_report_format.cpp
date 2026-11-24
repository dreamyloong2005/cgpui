#include "probe_report_support.hpp"
#include <ostream>

namespace renderer_probe {
void quote_json(std::ostream& out, std::string_view value) {
  constexpr char hex[] = "0123456789abcdef";
  out << '"';
  for (const unsigned char c : value) {
    if (c == '"' || c == '\\') out << '\\' << char(c);
    else if (c < 32) out << "\\u00" << hex[c >> 4] << hex[c & 15];
    else out << char(c);
  }
  out << '"';
}
void write_creation(std::ostream& out, const std::optional<RendererCreationSample>& creation) {
  if (!creation) { out << "null"; return; }
  const char* outcome = "exception";
  switch (creation->outcome) {
    case CreationOutcome::factory_error: outcome = "factory_error"; break;
    case CreationOutcome::null_renderer: outcome = "null_renderer"; break;
    case CreationOutcome::exception: break;
    case CreationOutcome::created: outcome = "created"; break;
  }
  out << "{\"wall_ns\":" << creation->wall_ns << ",\"outcome\":";
  quote_json(out, outcome); out << '}';
}
void write_identity(std::ostream& out, const cgpui::RendererBackendInfo& info) {
  out << "{\"backend\":"; quote_json(out, backend_name(info.backend));
  out << ",\"adapter_name\":"; quote_json(out, info.adapter_name);
  out << ",\"vendor_id\":" << info.vendor_id << ",\"luid_decimal\":";
  if (info.adapter_luid) quote_json(out, std::to_string(*info.adapter_luid)); else out << "null";
  out << ",\"software\":" << std::boolalpha << info.software_adapter
      << ",\"selection\":" << static_cast<int>(info.selection)
      << ",\"generation\":" << info.device_generation << ",\"fallback\":";
  quote_json(out, info.fallback_reason); out << '}';
}
void write_display(std::ostream& out, const std::optional<DisplayFacts>& facts) {
  if (!facts) { out << "null"; return; }
  const auto& f = *facts;
  out << "{\"device\":"; quote_json(out, f.device);
  out << ",\"valid\":" << std::boolalpha << f.valid << ",\"event_epoch\":" << f.event_epoch
      << ",\"dpi\":" << f.dpi << ",\"width\":" << f.width << ",\"height\":" << f.height
      << ",\"hz\":" << f.hz << ",\"bits_per_pixel\":" << f.bits_per_pixel
      << ",\"orientation\":" << f.orientation << ",\"monitor_left\":" << f.monitor_left
      << ",\"monitor_top\":" << f.monitor_top << ",\"monitor_right\":" << f.monitor_right
      << ",\"monitor_bottom\":" << f.monitor_bottom << ",\"window_left\":" << f.window_left
      << ",\"window_top\":" << f.window_top << '}';
}
void write_sample_row(std::ostream& out, const Sample& s, std::string_view phase) {
  out << phase << ',' << s.index << ',' << s.attempts << ',' << s.acquire_wall_ns << ','
      << s.begin_api_wall_ns << ',' << s.wait_wall_ns << ',' << s.record_wall_ns << ','
      << s.present_wall_ns << ',' << s.frame_wall_ns << ',' << s.stages.pacing_wait_nanoseconds << ','
      << s.stages.resource_preparation_nanoseconds << ',' << s.stages.image_acquisition_nanoseconds << ','
      << s.stages.command_recording_nanoseconds << ',' << s.stages.queue_submission_nanoseconds << ','
      << s.stages.presentation_nanoseconds << ',' << s.stages.total_nanoseconds << ','
      << s.commands << ',' << s.draws << ',' << s.batches << ',' << s.image_bytes << ',' << s.glyph_bytes << '\n';
}
} // namespace renderer_probe
