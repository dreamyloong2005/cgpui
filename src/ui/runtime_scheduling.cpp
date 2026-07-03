#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::request_render() {
  invalidation_state_.render = true;
  invalidation_state_.layout = true;
  invalidation_state_.paint = true;
  schedule_redraw();
}

void WindowRuntime::request_layout() {
  invalidation_state_.layout = true;
  invalidation_state_.paint = true;
  schedule_redraw();
}

void WindowRuntime::request_paint() {
  invalidation_state_.paint = true;
  schedule_redraw();
}

void WindowRuntime::defer(DeferredCallback callback) {
  if (!callback) {
    return;
  }
  deferred_callbacks_.push_back(std::move(callback));
  request_platform_wakeup();
}

TimerId WindowRuntime::schedule_timer(
    std::uint64_t delay_ms,
    TimerCallback callback) {
  if (!callback) {
    return {};
  }
  const TimerId id{next_timer_id_++};
  timers_.push_back(RuntimeTimer{
      .id = id,
      .due_ms = current_time_ms_ + delay_ms,
      .interval_ms = 0,
      .repeating = false,
      .callback = std::move(callback),
  });
  request_platform_wakeup();
  return id;
}

TimerId WindowRuntime::schedule_repeating_timer(
    std::uint64_t interval_ms,
    TimerCallback callback) {
  if (!callback || interval_ms == 0) {
    return {};
  }
  const TimerId id{next_timer_id_++};
  timers_.push_back(RuntimeTimer{
      .id = id,
      .due_ms = current_time_ms_ + interval_ms,
      .interval_ms = interval_ms,
      .repeating = true,
      .callback = std::move(callback),
  });
  request_platform_wakeup();
  return id;
}

bool WindowRuntime::cancel_timer(TimerId id) {
  if (id.value == 0) {
    return false;
  }
  const auto timer = std::find_if(
      timers_.begin(),
      timers_.end(),
      [id](const RuntimeTimer& timer) {
        return timer.id == id;
      });
  if (timer == timers_.end()) {
    return false;
  }
  timers_.erase(timer);
  return true;
}

void WindowRuntime::advance_time(std::uint64_t delta_ms) {
  current_time_ms_ += delta_ms;
  fire_due_timers();
  flush_deferred_redraw_request();
}

AnimationHandle WindowRuntime::start_animation(
    AnimationOptions options,
    AnimationCallback callback) {
  if (!callback) {
    return {};
  }

  if (options.tick_interval_ms == 0) {
    options.tick_interval_ms = 16;
  }

  const AnimationId id{next_animation_id_++};
  animations_.push_back(RuntimeAnimation{
      .id = id,
      .options = options,
      .callback = std::move(callback),
      .started_ms = current_time_ms_,
      .last_tick_ms = current_time_ms_,
      .complete = options.duration_ms == 0,
  });

  if (options.duration_ms != 0) {
    const TimerId timer_id = schedule_repeating_timer(
        options.tick_interval_ms,
        [this, id](const WindowRuntimeContext&) {
          tick_animation(id);
        });
    auto stored = std::find_if(
        animations_.begin(),
        animations_.end(),
        [id](const RuntimeAnimation& animation) {
          return animation.id == id;
        });
    if (stored != animations_.end()) {
      stored->timer_id = timer_id;
    }
  }

  return AnimationHandle(*this, id);
}

std::optional<AnimationSnapshot> WindowRuntime::animation_snapshot(
    AnimationId id) const {
  if (id.value == 0) {
    return std::nullopt;
  }

  const auto animation = std::find_if(
      animations_.begin(),
      animations_.end(),
      [id](const RuntimeAnimation& animation) {
        return animation.id == id;
      });
  if (animation == animations_.end()) {
    return std::nullopt;
  }

  const std::uint64_t duration_ms = animation->options.duration_ms;
  const std::uint64_t raw_elapsed_ms =
      current_time_ms_ >= animation->started_ms
          ? current_time_ms_ - animation->started_ms
          : 0;
  const std::uint64_t elapsed_ms =
      duration_ms == 0 ? 0 : std::min(raw_elapsed_ms, duration_ms);
  const float linear_progress =
      duration_ms == 0
          ? 1.0F
          : clamp_animation_progress(
                static_cast<float>(elapsed_ms) /
                static_cast<float>(duration_ms));
  const float eased_progress =
      ease(animation->options.easing, linear_progress);

  return AnimationSnapshot{
      .id = animation->id,
      .elapsed_ms = elapsed_ms,
      .duration_ms = duration_ms,
      .linear_progress = linear_progress,
      .eased_progress = eased_progress,
      .easing = animation->options.easing,
      .complete = animation->complete || linear_progress >= 1.0F,
  };
}

bool WindowRuntime::cancel_animation(AnimationId id) {
  if (id.value == 0) {
    return false;
  }

  const auto animation = std::find_if(
      animations_.begin(),
      animations_.end(),
      [id](const RuntimeAnimation& animation) {
        return animation.id == id;
      });
  if (animation == animations_.end()) {
    return false;
  }
  const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
  if (animation->complete || !snapshot.has_value() || snapshot->complete) {
    return false;
  }

  animation->complete = true;
  if (animation->timer_id.value != 0) {
    (void)cancel_timer(animation->timer_id);
    animation->timer_id = {};
  }
  return true;
}

TaskHandle WindowRuntime::spawn_task(TaskCompletionCallback callback) {
  if (!callback) {
    return {};
  }

  TaskId id;
  {
    std::lock_guard lock(tasks_mutex_);
    id = TaskId{next_task_id_++};
    tasks_.push_back(RuntimeTask{
        .id = id,
        .callback = std::move(callback),
        .queued = false,
        .completed = false,
    });
  }
  return TaskHandle(*this, id);
}

TaskHandle WindowRuntime::spawn_background_task(
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) {
  if (!work || !completion) {
    return {};
  }

  auto cancellation_requested = std::make_shared<std::atomic_bool>(false);
  TaskId id;
  {
    std::lock_guard lock(tasks_mutex_);
    id = TaskId{next_task_id_++};
    tasks_.push_back(RuntimeTask{
        .id = id,
        .callback = std::move(completion),
        .queued = false,
        .completed = false,
        .cancelled = false,
        .background = true,
        .cancellation_requested = cancellation_requested,
    });
  }

  std::jthread worker(
      [this,
       id,
       cancellation_requested,
       work = std::move(work)]() mutable {
        try {
          work(TaskCancellationToken(cancellation_requested));
        } catch (...) {
        }
        (void)complete_task(id);
      });

  {
    std::lock_guard lock(tasks_mutex_);
    const auto task = std::find_if(
        tasks_.begin(),
        tasks_.end(),
        [id](const RuntimeTask& task) {
          return task.id == id;
        });
    if (task != tasks_.end()) {
      task->worker = std::move(worker);
    }
  }

  return TaskHandle(*this, id);
}

bool WindowRuntime::complete_task(TaskId id) {
  if (id.value == 0) {
    return false;
  }

  {
    std::lock_guard lock(tasks_mutex_);
    const auto task = std::find_if(
        tasks_.begin(),
        tasks_.end(),
        [id](const RuntimeTask& task) {
          return task.id == id;
        });
    if (task == tasks_.end() || task->queued || task->completed ||
        task->cancelled) {
      return false;
    }

    task->queued = true;
    task_completion_queue_.push_back(id);
  }

  request_platform_wakeup();
  return true;
}

void WindowRuntime::drain_task_completions() {
  if (draining_task_completions_ || should_quit_) {
    return;
  }

  draining_task_completions_ = true;
  while (!should_quit_) {
    std::vector<TaskId> queued;
    {
      std::lock_guard lock(tasks_mutex_);
      if (task_completion_queue_.empty()) {
        break;
      }
      queued.swap(task_completion_queue_);
    }
    for (const TaskId id : queued) {
      TaskCompletionCallback callback;
      {
        std::lock_guard lock(tasks_mutex_);
        const auto task = std::find_if(
            tasks_.begin(),
            tasks_.end(),
            [id](const RuntimeTask& task) {
              return task.id == id;
            });
        if (task == tasks_.end() || task->completed || task->cancelled) {
          continue;
        }

        callback = task->callback;
        task->queued = false;
        task->completed = true;
      }
      if (callback) {
        callback(context());
      }
    }
  }
  draining_task_completions_ = false;
  flush_deferred_redraw_request();
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
  deferred_redraw_request_ = false;
}

InvalidationState WindowRuntime::invalidation_state() const {
  return invalidation_state_;
}


void WindowRuntime::schedule_redraw() {
  if (window_ == nullptr || redraw_scheduled_ || should_quit_) {
    return;
  }
  redraw_scheduled_ = true;
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
  if (window_ == nullptr || renderer_ == nullptr || should_quit_) {
    return;
  }
  handling_wakeup_ = true;
  drain_task_completions();
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

void WindowRuntime::fire_due_timers() {
  if (firing_timers_ || should_quit_) {
    return;
  }

  firing_timers_ = true;
  while (!should_quit_) {
    auto timer = std::find_if(
        timers_.begin(),
        timers_.end(),
        [this](const RuntimeTimer& timer) {
          return timer.callback && timer.due_ms <= current_time_ms_;
        });
    if (timer == timers_.end()) {
      break;
    }

    TimerCallback callback = timer->callback;
    if (timer->repeating) {
      timer->due_ms += timer->interval_ms;
    } else {
      timer = timers_.erase(timer);
    }
    if (callback) {
      callback(context());
    }
  }
  firing_timers_ = false;
}

void WindowRuntime::tick_animation(AnimationId id) {
  const auto animation = std::find_if(
      animations_.begin(),
      animations_.end(),
      [id](const RuntimeAnimation& animation) {
        return animation.id == id;
      });
  if (animation == animations_.end() || animation->complete ||
      animation->last_tick_ms == current_time_ms_) {
    return;
  }

  animation->last_tick_ms = current_time_ms_;
  const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
  if (!snapshot.has_value()) {
    return;
  }

  AnimationCallback callback = animation->callback;
  if (callback) {
    callback(context(), *snapshot);
  }

  if (snapshot->complete) {
    const auto completed = std::find_if(
        animations_.begin(),
        animations_.end(),
        [id](const RuntimeAnimation& animation) {
          return animation.id == id;
        });
    if (completed != animations_.end()) {
      completed->complete = true;
      if (completed->timer_id.value != 0) {
        (void)cancel_timer(completed->timer_id);
        completed->timer_id = {};
      }
    }
  }
}


bool WindowRuntime::task_active(TaskId id) const {
  if (id.value == 0) {
    return false;
  }

  std::lock_guard lock(tasks_mutex_);
  const auto task = std::find_if(
      tasks_.begin(),
      tasks_.end(),
      [id](const RuntimeTask& task) {
        return task.id == id;
      });
  return task != tasks_.end() && !task->queued && !task->completed &&
         !task->cancelled;
}

bool WindowRuntime::task_complete(TaskId id) const {
  if (id.value == 0) {
    return false;
  }

  std::lock_guard lock(tasks_mutex_);
  const auto task = std::find_if(
      tasks_.begin(),
      tasks_.end(),
      [id](const RuntimeTask& task) {
        return task.id == id;
      });
  return task != tasks_.end() && task->completed;
}

bool WindowRuntime::task_cancelled(TaskId id) const {
  if (id.value == 0) {
    return false;
  }

  std::lock_guard lock(tasks_mutex_);
  const auto task = std::find_if(
      tasks_.begin(),
      tasks_.end(),
      [id](const RuntimeTask& task) {
        return task.id == id;
      });
  return task != tasks_.end() && task->cancelled;
}

bool WindowRuntime::cancel_task(TaskId id) {
  if (id.value == 0) {
    return false;
  }

  std::lock_guard lock(tasks_mutex_);
  const auto task = std::find_if(
      tasks_.begin(),
      tasks_.end(),
      [id](const RuntimeTask& task) {
        return task.id == id;
      });
  if (task == tasks_.end() || task->completed || task->cancelled) {
    return false;
  }

  if (task->cancellation_requested != nullptr) {
    task->cancellation_requested->store(true);
  }
  task->cancelled = true;
  task->queued = false;
  task_completion_queue_.erase(
      std::remove(task_completion_queue_.begin(), task_completion_queue_.end(), id),
      task_completion_queue_.end());
  return true;
}


bool WindowRuntime::animation_active(AnimationId id) const {
  const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
  return snapshot.has_value() && !snapshot->complete;
}

bool WindowRuntime::animation_complete(AnimationId id) const {
  const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
  return snapshot.has_value() && snapshot->complete;
}


} // namespace cgpui
