#include "ui_internal.hpp"
#include <utility>

namespace cgpui {

bool WindowRuntime::task_active(TaskId id) const {
  if (id.value == 0) {
    return false;
  }

  std::lock_guard lock(state_->tasks.tasks_mutex_);
  const auto task = std::find_if(
      state_->tasks.tasks_.begin(),
      state_->tasks.tasks_.end(),
      [id](const RuntimeTask& task) {
        return task.id == id;
      });
  return task != state_->tasks.tasks_.end() && !task->queued && !task->completed &&
         !task->cancelled;
}

bool WindowRuntime::task_complete(TaskId id) const {
  if (id.value == 0) {
    return false;
  }

  std::lock_guard lock(state_->tasks.tasks_mutex_);
  const auto task = std::find_if(
      state_->tasks.tasks_.begin(),
      state_->tasks.tasks_.end(),
      [id](const RuntimeTask& task) {
        return task.id == id;
      });
  return task != state_->tasks.tasks_.end() && task->completed;
}

bool WindowRuntime::task_cancelled(TaskId id) const {
  if (id.value == 0) {
    return false;
  }

  std::lock_guard lock(state_->tasks.tasks_mutex_);
  const auto task = std::find_if(
      state_->tasks.tasks_.begin(),
      state_->tasks.tasks_.end(),
      [id](const RuntimeTask& task) {
        return task.id == id;
      });
  return task != state_->tasks.tasks_.end() && task->cancelled;
}

bool WindowRuntime::cancel_task(TaskId id) {
  if (id.value == 0) {
    return false;
  }

  // Captures may reenter the runtime when destroyed; release after unlocking.
  TaskCompletionCallback retired_callback;
  std::lock_guard lock(state_->tasks.tasks_mutex_);
  const auto task = std::find_if(
      state_->tasks.tasks_.begin(),
      state_->tasks.tasks_.end(),
      [id](const RuntimeTask& task) {
        return task.id == id;
      });
  if (task == state_->tasks.tasks_.end() || task->completed || task->cancelled) {
    return false;
  }

  if (task->cancellation_requested != nullptr) {
    task->cancellation_requested->store(true);
  }
  task->cancelled = true;
  task->queued = false;
  retired_callback = std::exchange(task->callback, {});
  state_->tasks.task_completion_queue_.erase(
      std::remove_if(
          state_->tasks.task_completion_queue_.begin(),
          state_->tasks.task_completion_queue_.end(),
          [id](const RuntimeTaskCompletion& completion) {
            return completion.id == id;
          }),
      state_->tasks.task_completion_queue_.end());
  return true;
}

} // namespace cgpui
