#include "ui_internal.hpp"

namespace cgpui {

int WindowRuntime::run(
    const WindowDescriptor& descriptor,
    WindowRuntimeOptions options) {
  should_quit_ = false;
  failed_ = false;
  window_ = nullptr;
  renderer_ = nullptr;
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(root_window_runtime_id_);
      root_record != nullptr) {
    root_record->descriptor = descriptor;
    root_record->root_view_id = root_view_id_;
    root_record->framebuffer_size = descriptor.size;
    root_record->viewport_size = descriptor.size;
    root_record->scale = {};
    root_record->redraw_scheduled = false;
    root_record->window = nullptr;
    root_record->renderer = nullptr;
    root_record->active = false;
  }
  framebuffer_size_ = descriptor.size;
  viewport_size_ = descriptor.size;
  scale_ = {};
  input_ = {};
  pointer_capture_owner_.reset();
  keyboard_focus_owner_.reset();
  keyboard_focus_element_owner_.reset();
  hovered_element_id_.reset();
  cursor_shape_ = CursorShape::default_arrow;
  applied_cursor_shape_ = CursorShape::default_arrow;
  last_event_result_ = EventResult::unhandled();
  last_render_record_.reset();
  last_frame_statistics_.reset();
  last_renderer_frame_diagnostics_.reset();
  last_event_dispatch_.reset();
  last_action_dispatch_.reset();
  current_event_route_.reset();
  invalidation_state_ = {};
  subscription_query_buffer_.clear();
  entity_count_ = 0;
  dispatching_view_event_ = false;
  firing_timers_ = false;
  draining_task_completions_ = false;
  handling_wakeup_ = false;
  update_batch_depth_ = 0;
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  rendering_frame_ = false;
  next_frame_redraw_requested_ = false;
  event_dispatch_sequence_ = 0;
  render_sequence_ = 0;
  frame_index_ = 0;
  applied_ime_text_input_placement_.reset();
  last_platform_accessibility_update_.reset();
  platform_diagnostics_.clear();
  platform_diagnostic_sequence_ = 0;

  auto window_result = application_.create_window(
      descriptor,
      [this](const PlatformEvent& event) { handle_event(event); });
  if (!window_result) {
    if (error_callback_) {
      error_callback_(window_result.error());
    }
    return 1;
  }

  std::unique_ptr<PlatformWindow> window = std::move(*window_result);
  window_ = window.get();
  const WindowState window_state = window_->state();
  framebuffer_size_ = window_state.framebuffer_size;
  scale_ = window_state.scale;
  viewport_size_ = to_logical_pixels(framebuffer_size_, scale_);

  auto renderer_result = try_create_renderer(RenderSurfaceDescriptor{
      .native_surface = window_->native_surface(),
      .framebuffer_size = window_state.framebuffer_size,
      .scale = window_state.scale,
      .transparent_background = descriptor.chrome.transparent_background});
  if (!renderer_result) {
    if (error_callback_) {
      error_callback_(renderer_result.error());
    }
    return 1;
  }
  renderer_ = *renderer_result;
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(root_window_runtime_id_);
      root_record != nullptr) {
    root_record->window = window_;
    root_record->renderer = renderer_;
    root_record->framebuffer_size = framebuffer_size_;
    root_record->viewport_size = viewport_size_;
    root_record->scale = scale_;
    root_record->active = true;
  }
  activate_pending_native_windows();

  if (options.request_initial_redraw) {
    redraw_scheduled_ = true;
    set_root_redraw_scheduled(true);
    window_->request_redraw();
  }

  const int run_result = application_.run();
  collect_retired_native_windows();
  deactivate_native_additional_windows();
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(root_window_runtime_id_);
      root_record != nullptr) {
    root_record->window = nullptr;
    root_record->renderer = nullptr;
    root_record->active = false;
  }
  window_ = nullptr;
  renderer_ = nullptr;

  if (failed_) {
    return 1;
  }
  return run_result;
}

void WindowRuntime::fail_and_quit(Error error) {
  failed_ = true;
  should_quit_ = true;
  if (error_callback_) {
    error_callback_(error);
  }
  application_.quit();
}

} // namespace cgpui
