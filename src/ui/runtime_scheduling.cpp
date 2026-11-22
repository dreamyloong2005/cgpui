#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::request_render() {
  state_->scheduling.invalidation_state_.render = true;
  state_->scheduling.invalidation_state_.layout = true;
  state_->scheduling.invalidation_state_.paint = true;
  notify_window_observers();
  notify_view_observers(state_->views.root_view_id_);
  schedule_redraw();
}

void WindowRuntime::request_layout() {
  state_->scheduling.invalidation_state_.layout = true;
  state_->scheduling.invalidation_state_.paint = true;
  notify_window_observers();
  notify_view_observers(state_->views.root_view_id_);
  schedule_redraw();
}

void WindowRuntime::request_paint() {
  state_->scheduling.invalidation_state_.paint = true;
  notify_window_observers();
  notify_view_observers(state_->views.root_view_id_);
  schedule_redraw();
}

void WindowRuntime::defer(DeferredCallback callback) {
  if (!callback || state_->core.shutting_down_) {
    return;
  }
  state_->scheduling.deferred_callbacks_.push_back(std::move(callback));
  request_platform_wakeup();
}

void WindowRuntime::batch_updates(UpdateBatchCallback callback) {
  if (!callback || state_->core.should_quit_) {
    return;
  }

  state_->scheduling.update_batch_depth_ += 1;
  callback(context());
  state_->scheduling.update_batch_depth_ -= 1;
  if (state_->scheduling.update_batch_depth_ == 0) {
    flush_deferred_redraw_request();
  }
}

void WindowRuntime::clear_invalidation() {
  state_->scheduling.invalidation_state_ = {};
  state_->scheduling.redraw_scheduled_ = false;
  set_root_redraw_scheduled(false);
  state_->scheduling.deferred_redraw_request_ = false;
  state_->scheduling.next_frame_redraw_requested_ = false;
}

InvalidationState WindowRuntime::invalidation_state() const {
  return state_->scheduling.invalidation_state_;
}


void WindowRuntime::schedule_redraw() {
  if (state_->windows.window_ == nullptr || state_->core.should_quit_) {
    return;
  }
  if (state_->scheduling.rendering_frame_) {
    state_->scheduling.next_frame_redraw_requested_ = true;
    return;
  }
  if (state_->scheduling.redraw_scheduled_) {
    return;
  }
  state_->scheduling.redraw_scheduled_ = true;
  set_root_redraw_scheduled(true);
  if (state_->input.dispatching_view_event_ || state_->scheduling.draining_deferred_callbacks_ || state_->scheduling.firing_timers_ ||
      state_->tasks.draining_task_completions_ || state_->scheduling.handling_wakeup_ || state_->scheduling.update_batch_depth_ > 0) {
    state_->scheduling.deferred_redraw_request_ = true;
    return;
  }
  state_->windows.window_->request_redraw();
}

void WindowRuntime::flush_deferred_redraw_request() {
  if (!state_->scheduling.deferred_redraw_request_ || state_->scheduling.handling_wakeup_ || state_->windows.window_ == nullptr ||
      state_->core.should_quit_) {
    return;
  }
  state_->scheduling.deferred_redraw_request_ = false;
  state_->windows.window_->request_redraw();
}

void WindowRuntime::request_platform_wakeup() {
  if (state_->windows.window_ == nullptr || state_->core.should_quit_) {
    return;
  }
  state_->core.application_.request_wakeup();
}

void WindowRuntime::handle_wakeup() {
  collect_retired_native_windows();
  if (state_->windows.window_ == nullptr || state_->windows.renderer_ == nullptr || state_->core.should_quit_) {
    return;
  }
  state_->scheduling.handling_wakeup_ = true;
  sync_platform_time();
  drain_async_io_completions();
  drain_task_completions();
  drain_cross_thread_entity_operations();
  fire_due_timers();
  drain_deferred_callbacks();
  state_->scheduling.handling_wakeup_ = false;
  flush_deferred_redraw_request();
}

void WindowRuntime::drain_deferred_callbacks() {
  while (!state_->scheduling.deferred_callbacks_.empty() && !state_->core.should_quit_) {
    std::vector<DeferredCallback> callbacks;
    callbacks.swap(state_->scheduling.deferred_callbacks_);
    state_->scheduling.draining_deferred_callbacks_ = true;
    for (DeferredCallback& callback : callbacks) {
      if (callback) {
        callback(context());
      }
    }
    state_->scheduling.draining_deferred_callbacks_ = false;
  }
}

} // namespace cgpui
