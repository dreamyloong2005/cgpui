#pragma once

#include "cgpui/ui/window_runtime.hpp"
#include "runtime_state_core_types_internal.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeCoreState {
  RuntimeCoreState(PlatformApplication& application, View& view, RendererFactory factory);
  PlatformApplication& application_;
  View& view_;
  RendererFactory renderer_factory_;
  WindowRuntimeFrameCallback after_frame_callback_;
  WindowRuntimeRenderCallback after_render_callback_;
  WindowRuntimeEventCallback after_event_callback_;
  WindowRuntimeFrameCallback close_requested_callback_;
  WindowRuntimeErrorCallback error_callback_;
  bool should_quit_ = false;
  bool shutting_down_ = false;
  bool failed_ = false;
};

} // namespace cgpui
