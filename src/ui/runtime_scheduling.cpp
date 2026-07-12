#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::request_render() {
  invalidation_state_.render = true;
  invalidation_state_.layout = true;
  invalidation_state_.paint = true;
  notify_window_observers();
  notify_view_observers(root_view_id_);
  schedule_redraw();
}

void WindowRuntime::request_layout() {
  invalidation_state_.layout = true;
  invalidation_state_.paint = true;
  notify_window_observers();
  notify_view_observers(root_view_id_);
  schedule_redraw();
}

void WindowRuntime::request_paint() {
  invalidation_state_.paint = true;
  notify_window_observers();
  notify_view_observers(root_view_id_);
  schedule_redraw();
}

void WindowRuntime::defer(DeferredCallback callback) {
  if (!callback) {
    return;
  }
  deferred_callbacks_.push_back(std::move(callback));
  request_platform_wakeup();
}

void WindowRuntime::batch_updates(UpdateBatchCallback callback) {
  if (!callback || should_quit_) {
    return;
  }

  update_batch_depth_ += 1;
  callback(context());
  update_batch_depth_ -= 1;
  if (update_batch_depth_ == 0) {
    flush_deferred_redraw_request();
  }
}

void WindowRuntime::clear_invalidation() {
  invalidation_state_ = {};
  redraw_scheduled_ = false;
  set_root_redraw_scheduled(false);
  deferred_redraw_request_ = false;
  next_frame_redraw_requested_ = false;
}

InvalidationState WindowRuntime::invalidation_state() const {
  return invalidation_state_;
}


void WindowRuntime::schedule_redraw() {
  if (window_ == nullptr || should_quit_) {
    return;
  }
  if (rendering_frame_) {
    next_frame_redraw_requested_ = true;
    return;
  }
  if (redraw_scheduled_) {
    return;
  }
  redraw_scheduled_ = true;
  set_root_redraw_scheduled(true);
  if (dispatching_view_event_ || draining_deferred_callbacks_ || firing_timers_ ||
      draining_task_completions_ || handling_wakeup_ || update_batch_depth_ > 0) {
    deferred_redraw_request_ = true;
    return;
  }
  window_->request_redraw();
}

void WindowRuntime::flush_deferred_redraw_request() {
  if (!deferred_redraw_request_ || handling_wakeup_ || window_ == nullptr ||
      should_quit_) {
    return;
  }
  deferred_redraw_request_ = false;
  window_->request_redraw();
}

void WindowRuntime::request_platform_wakeup() {
  if (window_ == nullptr || should_quit_) {
    return;
  }
  application_.request_wakeup();
}

void WindowRuntime::handle_wakeup() {
  collect_retired_native_windows();
  if (window_ == nullptr || renderer_ == nullptr || should_quit_) {
    return;
  }
  handling_wakeup_ = true;
  sync_platform_time();
  drain_async_io_completions();
  drain_task_completions();
  drain_cross_thread_entity_operations();
  fire_due_timers();
  drain_deferred_callbacks();
  handling_wakeup_ = false;
  flush_deferred_redraw_request();
}

void WindowRuntime::drain_deferred_callbacks() {
  while (!deferred_callbacks_.empty() && !should_quit_) {
    std::vector<DeferredCallback> callbacks;
    callbacks.swap(deferred_callbacks_);
    draining_deferred_callbacks_ = true;
    for (DeferredCallback& callback : callbacks) {
      if (callback) {
        callback(context());
      }
    }
    draining_deferred_callbacks_ = false;
  }
}

} // namespace cgpui
