#include "ui_internal.hpp"

namespace cgpui {

WindowRuntimeContext WindowRuntime::context() {
  ViewInputState input = input_state();

  return WindowRuntimeContext{
      .runtime = *this,
      .application = application_,
      .window = *window_,
      .renderer = *renderer_,
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
