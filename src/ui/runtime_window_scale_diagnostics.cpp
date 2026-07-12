#include "runtime_window_scale_diagnostics_internal.hpp"

namespace cgpui {

PlatformDiagnosticEvent window_scale_diagnostic(
    const WindowResized& event,
    bool succeeded) {
  return PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::window_scale,
      .event_kind = EventKind::window_resized,
      .backend = "runtime",
      .operation = "resize-scale",
      .supported = true,
      .succeeded = succeeded,
      .framebuffer_size = event.size,
      .scale = event.scale,
  };
}

} // namespace cgpui
