#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::layout_element_tree_with_animations(
    FrameStatistics& frame_statistics) {
  const std::uint64_t scope_id = state_->windows.root_window_runtime_id_.value;
  state_->animations.element_animation_state_store_.begin_frame(scope_id, state_->scheduling.current_time_ms_);
  if (state_->elements.owned_element_tree_ != nullptr) {
    (void)state_->elements.owned_element_tree_->layout_root(LayoutInput{
        .constraints = {.max_size = state_->windows.viewport_size_},
        .scale = state_->windows.scale_,
        .animation_state_store = &state_->animations.element_animation_state_store_,
        .animation_scope_id = scope_id,
        .animation_time_ms = state_->scheduling.current_time_ms_,
    });
    frame_statistics.layout_pass_count += 1;
  }

  const ElementAnimationFrameResult result =
      state_->animations.element_animation_state_store_.finish_frame(scope_id);
  if (!result.requests_next_frame()) {
    clear_element_animation_frame();
    return;
  }
  request_element_animation_frame(result.next_frame_delay_ms);
}

} // namespace cgpui
