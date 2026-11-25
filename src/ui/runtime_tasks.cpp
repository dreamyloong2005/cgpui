#include "ui_internal.hpp"
#include "runtime_task_priority_internal.hpp"
#include <utility>

namespace cgpui {

TaskHandle WindowRuntime::spawn_task(TaskCompletionCallback callback) {
  return spawn_task(TaskPriority::normal, std::move(callback));
}

TaskHandle WindowRuntime::spawn_background_task(
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) {
  return spawn_background_task(
      TaskPriority::normal, std::move(work), std::move(completion));
}

bool WindowRuntime::complete_task(TaskId id) {
  if (id.value == 0) {
    return false;
  }

  {
    std::lock_guard lock(state_->tasks.tasks_mutex_);
    const auto task = std::find_if(
        state_->tasks.tasks_.begin(),
        state_->tasks.tasks_.end(),
        [id](const RuntimeTask& task) {
          return task.id == id;
        });
    if (task == state_->tasks.tasks_.end() || task->queued || task->completed ||
        task->cancelled) {
      return false;
    }

    task->queued = true;
    state_->tasks.task_completion_queue_.push_back(RuntimeTaskCompletion{
        .id = id,
        .priority = task->priority,
    });
  }

  request_platform_wakeup();
  return true;
}

void WindowRuntime::drain_task_completions() {
  if (state_->tasks.draining_task_completions_ || state_->core.should_quit_) {
    return;
  }

  state_->tasks.draining_task_completions_ = true;
  while (!state_->core.should_quit_) {
    std::vector<RuntimeTaskCompletion> queued;
    {
      std::lock_guard lock(state_->tasks.tasks_mutex_);
      if (state_->tasks.task_completion_queue_.empty()) {
        break;
      }
      queued.swap(state_->tasks.task_completion_queue_);
    }
    for (const TaskPriority priority : runtime_task_priorities_descending) {
      for (const RuntimeTaskCompletion& completion : queued) {
        if (completion.priority != priority) continue;
        const TaskId id = completion.id;
        TaskCompletionCallback callback;
        {
          std::lock_guard lock(state_->tasks.tasks_mutex_);
          const auto task = std::find_if(
              state_->tasks.tasks_.begin(),
              state_->tasks.tasks_.end(),
              [id](const RuntimeTask& task) {
                return task.id == id;
              });
          if (task == state_->tasks.tasks_.end() || task->completed || task->cancelled) {
            continue;
          }

          callback = std::exchange(task->callback, {});
          task->queued = false;
          task->completed = true;
        }
        record_platform_diagnostic(PlatformDiagnosticEvent{
            .kind = PlatformDiagnosticKind::scheduling,
            .backend = "runtime",
            .operation = "task-completed",
            .supported = true, .succeeded = true,
            .value_count = static_cast<std::size_t>(id.value),
        });
        if (callback) {
          callback(context());
        }
      }
    }
  }
  state_->tasks.draining_task_completions_ = false;
  flush_deferred_redraw_request();
}

} // namespace cgpui
