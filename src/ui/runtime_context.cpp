#include "runtime_detached_targets_internal.hpp"
#include "ui_internal.hpp"

namespace cgpui {

WindowRuntimeContext WindowRuntime::context() {
  ViewInputState input = input_state();

  return WindowRuntimeContext{
      .runtime = *this,
      .application = application_,
      // No window yet (or any more): callbacks still get a context
      // (UBSan: a reference bound to *nullptr before run()).
      .platform_window =
          window_ != nullptr ? *window_ : detached_runtime_window(),
      .renderer = renderer_ != nullptr ? *renderer_ : detached_runtime_renderer(),
      .window_runtime_id = root_window_runtime_id_,
      .view_id = root_view_id_,
      .viewport_size = viewport_size_,
      .scale = scale_,
      .input = input,
      .event_route = current_event_route_,
      .last_event_result = last_event_result_,
      .last_event_dispatch = last_event_dispatch_,
      .frame_index = frame_index_};
}

} // namespace cgpui
