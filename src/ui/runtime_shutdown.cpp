#include "ui_internal.hpp"
#include "runtime_async_io_internal.hpp"
#include "runtime_cross_thread_entity_internal.hpp"
#include "runtime_task_group_internal.hpp"
#include "runtime_task_pool_internal.hpp"
#include "runtime_window_store_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

WindowRuntime::~WindowRuntime() {
  state_->core.shutting_down_ = true;
  state_->core.should_quit_ = true;
  state_->scheduling.frame_readiness_.clear_all(*this);
  if (state_->platform.system_appearance_bridge_installed_) {
    state_->core.application_.set_system_appearance_callback({});
  }
  state_->tasks.cross_thread_entity_queue_->shutdown();
  std::vector<TaskCompletionCallback> retired_callbacks;
  {
    std::lock_guard lock(state_->tasks.tasks_mutex_);
    for (RuntimeTask& task : state_->tasks.tasks_) {
      if (task.cancellation_requested != nullptr) {
        task.cancellation_requested->store(true);
      }
      if (!task.completed) {
        task.cancelled = true;
      }
      retired_callbacks.push_back(std::move(task.callback));
    }
    state_->tasks.task_completion_queue_.clear();
  }
  state_->tasks.task_pool_->shutdown();
  state_->tasks.async_io_registry_->shutdown();
  deactivate_native_additional_windows();
  reset_root_window_resources();
  state_->windows.window_lifetime_->retire_remaining_renderers();
  collect_retired_native_windows();
  retired_callbacks.clear();
  // A user destructor may register another callback/global. Drain coherent
  // detached batches until no captured resources remain, before token detach.
  while (retire_runtime_user_resources()) {}
  state_->tasks.task_group_store_->detach_handles();
}

} // namespace cgpui
