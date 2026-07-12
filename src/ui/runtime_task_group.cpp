#include "ui_internal.hpp"
#include "runtime_task_group_internal.hpp"
#include "runtime_task_pool_internal.hpp"

#include <algorithm>
#include <atomic>
#include <memory>

namespace cgpui {

TaskGroupId WindowRuntime::RuntimeTaskGroupStore::create(
    WindowRuntime& runtime) {
  std::lock_guard lock(runtime.tasks_mutex_);
  const TaskGroupId id{next_id_++};
  records_.push_back(Record{.id = id});
  return id;
}

TaskHandle WindowRuntime::RuntimeTaskGroupStore::spawn_task(
    WindowRuntime& runtime,
    TaskGroupId group_id,
    TaskPriority priority,
    TaskCompletionCallback callback) {
  if (!callback || group_id.value == 0) return {};
  TaskId id;
  {
    std::lock_guard lock(runtime.tasks_mutex_);
    Record* group = find(group_id);
    if (group == nullptr || group->cancelled) return {};
    id = TaskId{runtime.next_task_id_++};
    runtime.tasks_.push_back(WindowRuntime::RuntimeTask{
        .id = id,
        .group_id = group_id,
        .priority = priority,
        .callback = std::move(callback),
    });
    group->task_ids.push_back(id);
  }
  return runtime.make_task_handle(id);
}

TaskHandle WindowRuntime::RuntimeTaskGroupStore::spawn_background_task(
    WindowRuntime& runtime,
    TaskGroupId group_id,
    TaskPriority priority,
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) {
  if (!work || !completion || group_id.value == 0) return {};
  auto cancellation_requested = std::make_shared<std::atomic_bool>(false);
  TaskId id;
  {
    std::lock_guard lock(runtime.tasks_mutex_);
    Record* group = find(group_id);
    if (group == nullptr || group->cancelled) return {};
    id = TaskId{runtime.next_task_id_++};
    runtime.tasks_.push_back(WindowRuntime::RuntimeTask{
        .id = id,
        .group_id = group_id,
        .priority = priority,
        .callback = std::move(completion),
        .background = true,
        .cancellation_requested = cancellation_requested,
    });
    group->task_ids.push_back(id);
  }
  const bool submitted = runtime.task_pool_->submit(
      priority,
      [&runtime, id, cancellation_requested, work = std::move(work)]() mutable {
        try {
          if (!cancellation_requested->load()) {
            work(runtime.make_task_cancellation_token(cancellation_requested));
          }
        } catch (...) {
        }
        (void)runtime.complete_task(id);
      });
  if (!submitted) {
    (void)runtime.cancel_task(id);
    return {};
  }
  return runtime.make_task_handle(id);
}

TaskGroup WindowRuntime::create_task_group() {
  return TaskGroup(*this, task_group_store_->create(*this));
}

TaskHandle WindowRuntime::make_task_handle(TaskId id) {
  return TaskHandle(*this, id);
}

TaskCancellationToken WindowRuntime::make_task_cancellation_token(
    std::shared_ptr<std::atomic_bool> state) {
  return TaskCancellationToken(std::move(state));
}

} // namespace cgpui
