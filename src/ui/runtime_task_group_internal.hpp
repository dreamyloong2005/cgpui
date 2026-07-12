#pragma once

#include "cgpui/ui/task_group.hpp"

#include <cstdint>
#include <vector>

namespace cgpui {

class WindowRuntime::RuntimeTaskGroupStore {
 public:
  [[nodiscard]] TaskGroupId create(WindowRuntime& runtime);
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
    std::vector<TaskId> task_ids;
    bool cancelled = false;
  };

  std::vector<Record> records_;
  std::uint64_t next_id_ = 1;
};

} // namespace cgpui
