#include "ui_internal.hpp"

namespace cgpui {

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

} // namespace cgpui
