#include "ui_internal.hpp"
#include "runtime_task_group_internal.hpp"
#include "runtime_task_pool_internal.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <ranges>

namespace cgpui {
namespace {

auto find_group(auto& records, TaskGroupId id) {
  return std::find_if(records.begin(), records.end(), [id](const auto& record) {
    return record.id == id;
  });
}

auto find_task(auto& tasks, TaskId id) {
  return std::find_if(tasks.begin(), tasks.end(), [id](const auto& task) {
    return task.id == id;
  });
}

} // namespace

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
    const auto group = find_group(records_, group_id);
    if (group == records_.end() || group->cancelled) return {};
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
    const auto group = find_group(records_, group_id);
    if (group == records_.end() || group->cancelled) return {};
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

std::size_t WindowRuntime::RuntimeTaskGroupStore::task_count(
    const WindowRuntime& runtime,
    TaskGroupId group_id) const {
  std::lock_guard lock(runtime.tasks_mutex_);
  const auto group = find_group(records_, group_id);
  return group == records_.end() ? 0 : group->task_ids.size();
}

std::size_t WindowRuntime::RuntimeTaskGroupStore::active_task_count(
    const WindowRuntime& runtime,
    TaskGroupId group_id) const {
  std::lock_guard lock(runtime.tasks_mutex_);
  const auto group = find_group(records_, group_id);
  if (group == records_.end()) return 0;
  std::size_t count = 0;
  for (const TaskId id : group->task_ids) {
    const auto task = find_task(runtime.tasks_, id);
    count += task != runtime.tasks_.end() && !task->completed && !task->cancelled;
  }
  return count;
}

bool WindowRuntime::RuntimeTaskGroupStore::complete(
    const WindowRuntime& runtime,
    TaskGroupId group_id) const {
  std::lock_guard lock(runtime.tasks_mutex_);
  const auto group = find_group(records_, group_id);
  if (group == records_.end()) return false;
  return std::ranges::all_of(group->task_ids, [&](TaskId id) {
    const auto task = find_task(runtime.tasks_, id);
    return task != runtime.tasks_.end() && (task->completed || task->cancelled);
  });
}

bool WindowRuntime::RuntimeTaskGroupStore::cancelled(
    const WindowRuntime& runtime,
    TaskGroupId group_id) const {
  std::lock_guard lock(runtime.tasks_mutex_);
  const auto group = find_group(records_, group_id);
  return group != records_.end() && group->cancelled;
}

bool WindowRuntime::RuntimeTaskGroupStore::cancel(
    WindowRuntime& runtime,
    TaskGroupId group_id) {
  std::lock_guard lock(runtime.tasks_mutex_);
  const auto group = find_group(records_, group_id);
  if (group == records_.end() || group->cancelled) return false;
  group->cancelled = true;
  for (const TaskId id : group->task_ids) {
    const auto task = find_task(runtime.tasks_, id);
    if (task == runtime.tasks_.end() || task->completed || task->cancelled) {
      continue;
    }
    if (task->cancellation_requested != nullptr) {
      task->cancellation_requested->store(true);
    }
    task->cancelled = true;
    task->queued = false;
  }
  std::erase_if(runtime.task_completion_queue_, [&](const auto& completion) {
    return std::ranges::find(group->task_ids, completion.id) !=
        group->task_ids.end();
  });
  return true;
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
