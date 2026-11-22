#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeDiagnosticState {
  std::optional<RenderRecord> last_render_record_;
  std::optional<EventDispatchRecord> last_event_dispatch_;
  std::optional<ActionDispatchResult> last_action_dispatch_;
  int event_dispatch_sequence_ = 0;
  int render_sequence_ = 0;
  int frame_index_ = 0;
  std::vector<PlatformDiagnosticEvent> platform_diagnostics_;
  int platform_diagnostic_sequence_ = 0;
  #include "runtime_renderer_diagnostic_state_internal.hpp"
};

} // namespace cgpui
