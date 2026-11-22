#include "ui_internal.hpp"
#include "runtime_task_pool_internal.hpp"

#include <expected>

namespace cgpui {

TaskHandle WindowRuntime::spawn_task(
    TaskPriority priority,
    TaskCompletionCallback callback) {
  if (!callback || state_->core.shutting_down_) return {};

  TaskId id;
  {
    std::lock_guard lock(state_->tasks.tasks_mutex_);
    id = TaskId{state_->tasks.next_task_id_++};
    state_->tasks.tasks_.push_back(RuntimeTask{
        .id = id,
        .priority = priority,
        .callback = std::move(callback),
    });
  }
  return TaskHandle(*this, id);
}

TaskHandle WindowRuntime::spawn_background_task(
    TaskPriority priority,
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) {
  if (!work || !completion || state_->core.shutting_down_) return {};

  auto cancellation_requested = std::make_shared<std::atomic_bool>(false);
  TaskId id;
  {
    std::lock_guard lock(state_->tasks.tasks_mutex_);
    id = TaskId{state_->tasks.next_task_id_++};
    state_->tasks.tasks_.push_back(RuntimeTask{
        .id = id,
        .priority = priority,
        .callback = std::move(completion),
        .background = true,
        .cancellation_requested = cancellation_requested,
    });
  }

  const bool submitted = state_->tasks.task_pool_->submit(
      priority,
      [this, id, cancellation_requested, work = std::move(work)]() mutable {
        try {
          if (!cancellation_requested->load()) {
            work(TaskCancellationToken(cancellation_requested));
          }
        } catch (...) {
        }
        (void)complete_task(id);
      });
  if (!submitted) {
    (void)cancel_task(id);
    return {};
  }
  return TaskHandle(*this, id);
}

Result<TaskHandle> WindowRuntime::try_spawn_task(
    TaskPriority priority,
    TaskCompletionCallback callback) {
  if (!callback) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "foreground task completion callback is required",
    });
  }
  return spawn_task(priority, std::move(callback));
}

Result<TaskHandle> WindowRuntime::try_spawn_background_task(
    TaskPriority priority,
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) {
  if (!work) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "background task work is required",
    });
  }
  if (!completion) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "background task completion callback is required",
    });
  }
  return spawn_background_task(
      priority,
      std::move(work),
      std::move(completion));
}

} // namespace cgpui
