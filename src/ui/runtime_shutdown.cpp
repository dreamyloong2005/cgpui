#include "ui_internal.hpp"

namespace cgpui {

WindowRuntime::~WindowRuntime() {
  std::vector<std::jthread> workers;
  {
    std::lock_guard lock(tasks_mutex_);
    for (RuntimeTask& task : tasks_) {
      if (task.cancellation_requested != nullptr) {
        task.cancellation_requested->store(true);
      }
      if (!task.completed) {
        task.cancelled = true;
      }
      if (task.worker.joinable()) {
        task.worker.request_stop();
        workers.push_back(std::move(task.worker));
      }
    }
    task_completion_queue_.clear();
  }
}

} // namespace cgpui
