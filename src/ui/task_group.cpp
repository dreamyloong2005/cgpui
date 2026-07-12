#include "cgpui/ui/task_group.hpp"

#include "ui_internal.hpp"
#include "runtime_task_group_internal.hpp"

#include <expected>
#include <utility>

namespace cgpui {

TaskGroup::~TaskGroup() { (void)cancel(); }

TaskGroup::TaskGroup(TaskGroup&& other) noexcept
    : runtime_(std::exchange(other.runtime_, nullptr)),
      id_(std::exchange(other.id_, {})) {}

TaskGroup& TaskGroup::operator=(TaskGroup&& other) noexcept {
  if (this == &other) return *this;
  (void)cancel();
  runtime_ = std::exchange(other.runtime_, nullptr);
  id_ = std::exchange(other.id_, {});
  return *this;
}

TaskGroup TaskGroup::create_child_group() const {
  if (runtime_ == nullptr) return {};
  const TaskGroupId child_id =
      runtime_->task_group_store_->create_child(*runtime_, id_);
  return child_id.value == 0 ? TaskGroup{} : TaskGroup(*runtime_, child_id);
}

std::size_t TaskGroup::task_count() const {
  return runtime_ == nullptr ? 0 :
      runtime_->task_group_store_->task_count(*runtime_, id_);
}

std::size_t TaskGroup::active_task_count() const {
  return runtime_ == nullptr ? 0 :
      runtime_->task_group_store_->active_task_count(*runtime_, id_);
}

bool TaskGroup::complete() const {
  return runtime_ != nullptr &&
      runtime_->task_group_store_->complete(*runtime_, id_);
}

bool TaskGroup::cancelled() const {
  return runtime_ != nullptr &&
      runtime_->task_group_store_->cancelled(*runtime_, id_);
}

bool TaskGroup::cancel() {
  return runtime_ != nullptr &&
      runtime_->task_group_store_->cancel(*runtime_, id_);
}

TaskHandle TaskGroup::spawn_task(TaskCompletionCallback callback) const {
  return spawn_task(TaskPriority::normal, std::move(callback));
}

TaskHandle TaskGroup::spawn_task(
    TaskPriority priority,
    TaskCompletionCallback callback) const {
  return runtime_ == nullptr ? TaskHandle{} :
      runtime_->task_group_store_->spawn_task(
          *runtime_, id_, priority, std::move(callback));
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
  return runtime_ == nullptr ? TaskHandle{} :
      runtime_->task_group_store_->spawn_background_task(
          *runtime_, id_, priority, std::move(work), std::move(completion));
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
