#include "runtime_detached_targets_internal.hpp"
#include "ui_internal.hpp"

namespace cgpui {

WindowRuntimeContext WindowRuntime::context() {
  ViewInputState input = input_state();

  return WindowRuntimeContext{
      .runtime = *this,
      .application = state_->core.application_,
      // No window yet (or any more): callbacks still get a context
      // (UBSan: a reference bound to *nullptr before run()).
      .platform_window =
          state_->windows.window_ != nullptr ? *state_->windows.window_ : detached_runtime_window(),
      .renderer = state_->windows.renderer_ != nullptr ? *state_->windows.renderer_ : detached_runtime_renderer(),
      .window_runtime_id = state_->windows.root_window_runtime_id_,
      .view_id = state_->views.root_view_id_,
      .viewport_size = state_->windows.viewport_size_,
      .scale = state_->windows.scale_,
      .input = input,
      .event_route = state_->input.current_event_route_,
      .last_event_result = state_->input.last_event_result_,
      .last_event_dispatch = state_->diagnostics.last_event_dispatch_,
      .frame_index = state_->diagnostics.frame_index_};
}

} // namespace cgpui
