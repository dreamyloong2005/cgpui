#include "cgpui/ui/task_group.hpp"

#include "ui_internal.hpp"
#include "runtime_task_group_internal.hpp"

#include <expected>
#include <utility>

namespace cgpui {

TaskGroup::~TaskGroup() { (void)cancel(); }

TaskGroup::TaskGroup(TaskGroup&& other) noexcept
    : runtime_(std::exchange(other.runtime_, nullptr)),
      runtime_lifetime_(std::move(other.runtime_lifetime_)),
      id_(std::exchange(other.id_, {})),
      window_runtime_id_(std::exchange(other.window_runtime_id_, {})) {}

TaskGroup& TaskGroup::operator=(TaskGroup&& other) noexcept {
  if (this == &other) return *this;
  (void)cancel();
  runtime_ = std::exchange(other.runtime_, nullptr);
  runtime_lifetime_ = std::move(other.runtime_lifetime_);
  id_ = std::exchange(other.id_, {});
  window_runtime_id_ = std::exchange(other.window_runtime_id_, {});
  return *this;
}

TaskGroup TaskGroup::create_child_group() const {
  auto* runtime = attached_runtime();
  if (runtime == nullptr) return {};
  const TaskGroupId child_id =
      runtime->state_->tasks.task_group_store_->create_child(*runtime, id_);
  if (child_id.value == 0) return {};
  TaskGroup child(*runtime, child_id);
  child.window_runtime_id_ = window_runtime_id_;
  return child;
}

std::size_t TaskGroup::task_count() const {
  auto* runtime = attached_runtime();
  return runtime == nullptr ? 0 :
      runtime->state_->tasks.task_group_store_->task_count(*runtime, id_);
}

std::size_t TaskGroup::active_task_count() const {
  auto* runtime = attached_runtime();
  return runtime == nullptr ? 0 :
      runtime->state_->tasks.task_group_store_->active_task_count(*runtime, id_);
}

bool TaskGroup::complete() const {
  auto* runtime = attached_runtime();
  return runtime != nullptr &&
      runtime->state_->tasks.task_group_store_->complete(*runtime, id_);
}

bool TaskGroup::cancelled() const {
  auto* runtime = attached_runtime();
  return runtime != nullptr &&
      runtime->state_->tasks.task_group_store_->cancelled(*runtime, id_);
}

bool TaskGroup::cancel() {
  auto* runtime = attached_runtime();
  return runtime != nullptr &&
      runtime->state_->tasks.task_group_store_->cancel(*runtime, id_);
}

TaskHandle TaskGroup::spawn_task(TaskCompletionCallback callback) const {
  return spawn_task(TaskPriority::normal, std::move(callback));
}

TaskHandle TaskGroup::spawn_task(
    TaskPriority priority,
    TaskCompletionCallback callback) const {
  auto* runtime = attached_runtime();
  return runtime == nullptr ? TaskHandle{} :
      runtime->state_->tasks.task_group_store_->spawn_task(
          *runtime, id_, priority, bind_callback(std::move(callback)));
}

Result<TaskHandle> TaskGroup::try_spawn_task(
    TaskCompletionCallback callback) const {
  return try_spawn_task(TaskPriority::normal, std::move(callback));
}

Result<TaskHandle> TaskGroup::try_spawn_task(
    TaskPriority priority,
    TaskCompletionCallback callback) const {
  if (!callback) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "task group completion callback is required",
    });
  }
  TaskHandle task = spawn_task(priority, std::move(callback));
  if (task.id().value != 0) return task;
  return std::unexpected(Error{
      .code = ErrorCode::invalid_argument,
      .message = "task group is unavailable",
  });
}

TaskHandle TaskGroup::spawn_background_task(
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  return spawn_background_task(
      TaskPriority::normal, std::move(work), std::move(completion));
}

TaskHandle TaskGroup::spawn_background_task(
    TaskPriority priority,
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  auto* runtime = attached_runtime();
  return runtime == nullptr ? TaskHandle{} :
      runtime->state_->tasks.task_group_store_->spawn_background_task(
          *runtime, id_, priority, std::move(work), bind_callback(std::move(completion)));
}

Result<TaskHandle> TaskGroup::try_spawn_background_task(
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  return try_spawn_background_task(
      TaskPriority::normal, std::move(work), std::move(completion));
}

Result<TaskHandle> TaskGroup::try_spawn_background_task(
    TaskPriority priority,
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  if (!work || !completion) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "task group background work and completion are required",
    });
  }
  TaskHandle task = spawn_background_task(
      priority, std::move(work), std::move(completion));
  if (task.id().value != 0) return task;
  return std::unexpected(Error{
      .code = ErrorCode::invalid_argument,
      .message = "task group is unavailable",
  });
}

} // namespace cgpui
