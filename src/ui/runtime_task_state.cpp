#include "ui_internal.hpp"

namespace cgpui {

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
      std::remove_if(
          task_completion_queue_.begin(),
          task_completion_queue_.end(),
          [id](const RuntimeTaskCompletion& completion) {
            return completion.id == id;
          }),
      task_completion_queue_.end());
  return true;
}

} // namespace cgpui
