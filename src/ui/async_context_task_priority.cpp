#include "cgpui/ui/async_context.hpp"

#include "cgpui/ui/runtime_context.hpp"

#include <utility>

namespace cgpui {

TaskHandle AsyncContextCapability::spawn_task(
    TaskPriority priority,
    TaskCompletionCallback callback) const {
  return context_->spawn_task(priority, std::move(callback));
}

Result<TaskHandle> AsyncContextCapability::try_spawn_task(
    TaskPriority priority,
    TaskCompletionCallback callback) const {
  return context_->try_spawn_task(priority, std::move(callback));
}

TaskHandle AsyncContextCapability::spawn_background_task(
    TaskPriority priority,
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  return context_->spawn_background_task(
      priority, std::move(work), std::move(completion));
}

Result<TaskHandle> AsyncContextCapability::try_spawn_background_task(
    TaskPriority priority,
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  return context_->try_spawn_background_task(
      priority, std::move(work), std::move(completion));
}

} // namespace cgpui
