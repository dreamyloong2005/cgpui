#include "ui_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

int WindowRuntime::run(
    const WindowDescriptor& descriptor,
    WindowRuntimeOptions options) {
  state_->core.should_quit_ = false;
  state_->core.failed_ = false;
  state_->windows.window_ = nullptr;
  state_->windows.renderer_ = nullptr;
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(state_->windows.root_window_runtime_id_);
      root_record != nullptr) {
    root_record->descriptor = descriptor;
    root_record->root_view_id = state_->views.root_view_id_;
    root_record->framebuffer_size = descriptor.size;
    root_record->viewport_size = descriptor.size;
    root_record->scale = {};
    root_record->redraw_scheduled = false;
    root_record->input = {};
    reset_event_state_for_record(*root_record);
    reset_accessibility_state_for_record(*root_record);
    root_record->window = nullptr;
    root_record->renderer = nullptr;
    root_record->active = false;
  }
  state_->windows.framebuffer_size_ = descriptor.size;
  state_->windows.viewport_size_ = descriptor.size;
  state_->windows.scale_ = {};
  state_->input.input_ = {};
  state_->input.pointer_capture_owner_.reset();
  state_->input.keyboard_focus_owner_.reset();
  state_->input.keyboard_focus_element_owner_.reset();
  state_->input.hovered_element_id_.reset();
  state_->input.cursor_shape_ = CursorShape::default_arrow;
  state_->input.applied_cursor_shape_ = CursorShape::default_arrow;
  state_->input.last_event_result_ = EventResult::unhandled();
  state_->diagnostics.last_render_record_.reset();
  state_->diagnostics.last_frame_statistics_.reset();
  state_->diagnostics.last_renderer_frame_diagnostics_.reset();
  state_->diagnostics.last_event_dispatch_.reset();
  state_->diagnostics.last_action_dispatch_.reset();
  state_->input.current_event_route_.reset();
  sync_root_event_record();
  state_->scheduling.invalidation_state_ = {};
  state_->entities.subscription_query_buffer_.clear();
  state_->entities.entity_count_ = 0;
  state_->input.dispatching_view_event_ = false;
  state_->scheduling.firing_timers_ = false;
  state_->tasks.draining_task_completions_ = false;
  state_->scheduling.handling_wakeup_ = false;
  state_->scheduling.update_batch_depth_ = 0;
  state_->scheduling.redraw_scheduled_ = false;
  state_->scheduling.deferred_redraw_request_ = false;
  state_->scheduling.rendering_frame_ = false;
  state_->scheduling.next_frame_redraw_requested_ = false;
  state_->diagnostics.event_dispatch_sequence_ = 0;
  state_->diagnostics.render_sequence_ = 0;
  state_->diagnostics.frame_index_ = 0;
  state_->text.applied_ime_text_input_placement_.reset();
  state_->diagnostics.platform_diagnostics_.clear();
  state_->diagnostics.platform_diagnostic_sequence_ = 0;
  sync_root_input_record();
  auto window_result = state_->core.application_.create_window(
      descriptor,
      [this](const PlatformEvent& event) { handle_event(event); });
  if (!window_result) {
    if (state_->core.error_callback_) {
      state_->core.error_callback_(window_result.error());
    }
    return 1;
  }

  std::unique_ptr<PlatformWindow> window = std::move(*window_result);
  RuntimeRootWindowScope root_lifetime(*this);
  state_->windows.window_ = window.get();
  const WindowState window_state = state_->windows.window_->state();
  state_->windows.framebuffer_size_ = window_state.framebuffer_size;
  state_->windows.scale_ = window_state.scale;
  state_->windows.viewport_size_ = to_logical_pixels(state_->windows.framebuffer_size_, state_->windows.scale_);
  auto renderer_result = try_create_renderer(RenderSurfaceDescriptor{
      .native_surface = state_->windows.window_->native_surface(),
      .framebuffer_size = window_state.framebuffer_size,
      .scale = window_state.scale,
      .transparent_background = runtime_wants_transparent_surface(descriptor)});
  if (!renderer_result) {
    if (state_->core.error_callback_) {
      state_->core.error_callback_(renderer_result.error());
    }
    return 1;
  }
  state_->windows.renderer_ = *renderer_result;
  (void)runtime_apply_window_backdrop(*state_->windows.window_, state_->windows.renderer_, descriptor.backdrop);
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(state_->windows.root_window_runtime_id_);
      root_record != nullptr) {
    root_record->window = state_->windows.window_;
    root_record->renderer = state_->windows.renderer_;
    root_record->owns_renderer = state_->windows.window_lifetime_->owns(state_->windows.renderer_);
    root_record->framebuffer_size = state_->windows.framebuffer_size_;
    root_record->viewport_size = state_->windows.viewport_size_;
    root_record->scale = state_->windows.scale_;
    root_record->active = true;
    root_record->input = input_state();
  }
  activate_pending_native_windows();
  schedule_animation_frame_wakeup(false);
  if (options.request_initial_redraw) {
    state_->scheduling.redraw_scheduled_ = true;
    set_root_redraw_scheduled(true);
    state_->windows.window_->request_redraw();
  }

  const int run_result = state_->core.application_.run();
  collect_retired_native_windows();
  deactivate_native_additional_windows();
  reset_root_window_resources();

  if (state_->core.failed_) {
    return 1;
  }
  return run_result;
}

void WindowRuntime::fail_and_quit(Error error) {
  state_->core.failed_ = true;
  state_->core.should_quit_ = true;
  if (state_->core.error_callback_) {
    state_->core.error_callback_(error);
  }
  state_->core.application_.quit();
}

} // namespace cgpui
