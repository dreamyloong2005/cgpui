#pragma once

#include "cgpui/ui/window_runtime.hpp"
#include "runtime_state_tasks_types_internal.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeTaskState {
  explicit RuntimeTaskState(WindowRuntime& owner);
  ~RuntimeTaskState();
  mutable std::mutex tasks_mutex_;
  std::shared_ptr<detail::CrossThreadEntityQueueState> cross_thread_entity_queue_;
  std::unique_ptr<RuntimeAsyncIoRegistry> async_io_registry_;
  std::unique_ptr<RuntimeTaskPool> task_pool_;
  std::unique_ptr<RuntimeTaskGroupStore> task_group_store_;
  std::vector<RuntimeTask> tasks_;
  std::vector<RuntimeTaskCompletion> task_completion_queue_;
  std::uint64_t next_task_id_ = 1;
  bool draining_task_completions_ = false;
};

} // namespace cgpui
