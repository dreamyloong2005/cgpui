#include "ui_internal.hpp"
#include "runtime_async_io_internal.hpp"
#include "runtime_cross_thread_entity_internal.hpp"
#include "runtime_task_group_internal.hpp"
#include "runtime_task_pool_internal.hpp"

namespace cgpui {

WindowRuntime::~WindowRuntime() {
  cross_thread_entity_queue_->shutdown();
  async_io_registry_->shutdown();
  {
    std::lock_guard lock(tasks_mutex_);
    for (RuntimeTask& task : tasks_) {
      if (task.cancellation_requested != nullptr) {
        task.cancellation_requested->store(true);
      }
      if (!task.completed) {
        task.cancelled = true;
      }
    }
    task_completion_queue_.clear();
  }
  task_pool_->shutdown();
}

} // namespace cgpui
