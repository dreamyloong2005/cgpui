#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::layout_element_tree_with_animations(
    FrameStatistics& frame_statistics) {
  const std::uint64_t scope_id = root_window_runtime_id_.value;
  element_animation_state_store_.begin_frame(scope_id, current_time_ms_);
  if (owned_element_tree_ != nullptr) {
    (void)owned_element_tree_->layout_root(LayoutInput{
        .constraints = {.max_size = viewport_size_},
        .scale = scale_,
        .animation_state_store = &element_animation_state_store_,
        .animation_scope_id = scope_id,
        .animation_time_ms = current_time_ms_,
    });
    frame_statistics.layout_pass_count += 1;
  }

  const ElementAnimationFrameResult result =
      element_animation_state_store_.finish_frame(scope_id);
  if (!result.requests_next_frame()) {
    if (element_animation_timer_id_.value != 0) {
      (void)cancel_timer(element_animation_timer_id_);
      element_animation_timer_id_ = {};
    }
    return;
  }
  if (element_animation_timer_id_.value != 0) return;

  element_animation_timer_id_ = schedule_timer(
      result.next_frame_delay_ms,
      [this](const WindowRuntimeContext&) {
        element_animation_timer_id_ = {};
        request_render();
      });
}

} // namespace cgpui
