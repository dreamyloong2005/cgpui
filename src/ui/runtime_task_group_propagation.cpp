#include "ui_internal.hpp"
#include "runtime_task_group_internal.hpp"

#include <algorithm>
#include <ranges>
#include <vector>

namespace cgpui {

WindowRuntime::RuntimeTaskGroupStore::Record*
WindowRuntime::RuntimeTaskGroupStore::find(TaskGroupId id) {
  const auto group = std::ranges::find(records_, id, &Record::id);
  return group == records_.end() ? nullptr : &*group;
}

const WindowRuntime::RuntimeTaskGroupStore::Record*
WindowRuntime::RuntimeTaskGroupStore::find(TaskGroupId id) const {
  const auto group = std::ranges::find(records_, id, &Record::id);
  return group == records_.end() ? nullptr : &*group;
}

TaskGroupId WindowRuntime::RuntimeTaskGroupStore::create_child(
    WindowRuntime& runtime,
    TaskGroupId parent_id) {
  std::lock_guard lock(runtime.tasks_mutex_);
  Record* parent = find(parent_id);
  if (parent == nullptr || parent->cancelled) return {};
  const TaskGroupId id{next_id_++};
  parent->child_ids.push_back(id);
  records_.push_back(Record{.id = id, .parent_id = parent_id});
  return id;
}

std::size_t WindowRuntime::RuntimeTaskGroupStore::task_count_locked(
    const WindowRuntime& runtime,
    const Record& group) const {
  (void)runtime;
  std::size_t count = group.task_ids.size();
  for (const TaskGroupId child_id : group.child_ids) {
    const Record* child = find(child_id);
    if (child != nullptr) count += task_count_locked(runtime, *child);
  }
  return count;
}

std::size_t WindowRuntime::RuntimeTaskGroupStore::active_task_count_locked(
    const WindowRuntime& runtime,
    const Record& group) const {
  std::size_t count = 0;
  for (const TaskId id : group.task_ids) {
    const auto task = std::ranges::find(runtime.tasks_, id, &WindowRuntime::RuntimeTask::id);
    count += task != runtime.tasks_.end() && !task->completed && !task->cancelled;
  }
  for (const TaskGroupId child_id : group.child_ids) {
    const Record* child = find(child_id);
    if (child != nullptr) count += active_task_count_locked(runtime, *child);
  }
  return count;
}

bool WindowRuntime::RuntimeTaskGroupStore::complete_locked(
    const WindowRuntime& runtime,
    const Record& group) const {
  const bool own_complete = std::ranges::all_of(group.task_ids, [&](TaskId id) {
    const auto task = std::ranges::find(runtime.tasks_, id, &WindowRuntime::RuntimeTask::id);
    return task != runtime.tasks_.end() && (task->completed || task->cancelled);
  });
  return own_complete && std::ranges::all_of(
      group.child_ids,
      [&](TaskGroupId child_id) {
        const Record* child = find(child_id);
        return child != nullptr && complete_locked(runtime, *child);
      });
}

std::size_t WindowRuntime::RuntimeTaskGroupStore::task_count(
    const WindowRuntime& runtime,
    TaskGroupId group_id) const {
  std::lock_guard lock(runtime.tasks_mutex_);
  const Record* group = find(group_id);
  return group == nullptr ? 0 : task_count_locked(runtime, *group);
}

std::size_t WindowRuntime::RuntimeTaskGroupStore::active_task_count(
    const WindowRuntime& runtime,
    TaskGroupId group_id) const {
  std::lock_guard lock(runtime.tasks_mutex_);
  const Record* group = find(group_id);
  return group == nullptr ? 0 : active_task_count_locked(runtime, *group);
}

bool WindowRuntime::RuntimeTaskGroupStore::complete(
    const WindowRuntime& runtime,
    TaskGroupId group_id) const {
  std::lock_guard lock(runtime.tasks_mutex_);
  const Record* group = find(group_id);
  return group != nullptr && complete_locked(runtime, *group);
}

bool WindowRuntime::RuntimeTaskGroupStore::cancelled(
    const WindowRuntime& runtime,
    TaskGroupId group_id) const {
  std::lock_guard lock(runtime.tasks_mutex_);
  const Record* group = find(group_id);
  return group != nullptr && group->cancelled;
}

void WindowRuntime::RuntimeTaskGroupStore::cancel_locked(
    WindowRuntime& runtime,
    Record& group,
    std::vector<TaskId>& cancelled_ids) {
  group.cancelled = true;
  for (const TaskId id : group.task_ids) {
    const auto task = std::ranges::find(runtime.tasks_, id, &WindowRuntime::RuntimeTask::id);
    if (task == runtime.tasks_.end() || task->completed || task->cancelled) continue;
    if (task->cancellation_requested != nullptr) {
      task->cancellation_requested->store(true);
    }
    task->cancelled = true;
    task->queued = false;
    cancelled_ids.push_back(id);
  }
  for (const TaskGroupId child_id : group.child_ids) {
    Record* child = find(child_id);
    if (child != nullptr && !child->cancelled) {
      cancel_locked(runtime, *child, cancelled_ids);
    }
  }
}

bool WindowRuntime::RuntimeTaskGroupStore::cancel(
    WindowRuntime& runtime,
    TaskGroupId group_id) {
  std::lock_guard lock(runtime.tasks_mutex_);
  Record* group = find(group_id);
  if (group == nullptr || group->cancelled) return false;
  std::vector<TaskId> cancelled_ids;
  cancel_locked(runtime, *group, cancelled_ids);
  std::erase_if(runtime.task_completion_queue_, [&](const auto& completion) {
    return std::ranges::find(cancelled_ids, completion.id) != cancelled_ids.end();
  });
  return true;
}

} // namespace cgpui
