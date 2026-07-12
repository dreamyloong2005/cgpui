#pragma once

#include "cgpui/ui/task_group.hpp"

#include <cstdint>
#include <vector>

namespace cgpui {

class WindowRuntime::RuntimeTaskGroupStore {
 public:
  [[nodiscard]] TaskGroupId create(WindowRuntime& runtime);
  [[nodiscard]] TaskGroupId create_child(
      WindowRuntime& runtime,
      TaskGroupId parent_id);
  [[nodiscard]] TaskHandle spawn_task(
      WindowRuntime& runtime,
      TaskGroupId group_id,
      TaskPriority priority,
      TaskCompletionCallback callback);
  [[nodiscard]] TaskHandle spawn_background_task(
      WindowRuntime& runtime,
      TaskGroupId group_id,
      TaskPriority priority,
      BackgroundTaskCallback work,
      TaskCompletionCallback completion);
  [[nodiscard]] std::size_t task_count(
      const WindowRuntime& runtime,
      TaskGroupId group_id) const;
  [[nodiscard]] std::size_t active_task_count(
      const WindowRuntime& runtime,
      TaskGroupId group_id) const;
  [[nodiscard]] bool complete(
      const WindowRuntime& runtime,
      TaskGroupId group_id) const;
  [[nodiscard]] bool cancelled(
      const WindowRuntime& runtime,
      TaskGroupId group_id) const;
  [[nodiscard]] bool cancel(WindowRuntime& runtime, TaskGroupId group_id);

 private:
  struct Record {
    TaskGroupId id;
    TaskGroupId parent_id{};
    std::vector<TaskGroupId> child_ids;
    std::vector<TaskId> task_ids;
    bool cancelled = false;
  };

  [[nodiscard]] Record* find(TaskGroupId id);
  [[nodiscard]] const Record* find(TaskGroupId id) const;
  [[nodiscard]] std::size_t task_count_locked(
      const WindowRuntime& runtime,
      const Record& group) const;
  [[nodiscard]] std::size_t active_task_count_locked(
      const WindowRuntime& runtime,
      const Record& group) const;
  [[nodiscard]] bool complete_locked(
      const WindowRuntime& runtime,
      const Record& group) const;
  void cancel_locked(
      WindowRuntime& runtime,
      Record& group,
      std::vector<TaskId>& cancelled_ids);

  std::vector<Record> records_;
  std::uint64_t next_id_ = 1;
};

} // namespace cgpui
