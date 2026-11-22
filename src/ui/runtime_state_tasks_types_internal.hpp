#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeTask {
  TaskId id; TaskGroupId group_id{};
  TaskPriority priority = TaskPriority::normal;
  TaskCompletionCallback callback;
  bool queued = false, completed = false;
  bool cancelled = false, background = false;
  std::shared_ptr<std::atomic_bool> cancellation_requested;
};

struct WindowRuntime::RuntimeTaskCompletion { TaskId id; TaskPriority priority; };

struct WindowRuntime::RuntimeTaskDiagnostics {
  std::size_t task_count = 0;
  std::size_t active_task_count = 0;
  std::size_t queued_task_count = 0;
  std::size_t completed_task_count = 0;
  std::size_t cancelled_task_count = 0;
  std::size_t background_task_count = 0;
};

} // namespace cgpui
