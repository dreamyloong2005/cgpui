#include "ui_internal.hpp"

#include <utility>

namespace cgpui {

TaskHandle WindowRuntimeContext::spawn_task(
    TaskPriority priority,
    TaskCompletionCallback callback) const {
  return runtime.spawn_task(priority, std::move(callback));
}

Result<TaskHandle> WindowRuntimeContext::try_spawn_task(
    TaskPriority priority,
    TaskCompletionCallback callback) const {
  return runtime.try_spawn_task(priority, std::move(callback));
}

TaskHandle WindowRuntimeContext::spawn_background_task(
    TaskPriority priority,
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  return runtime.spawn_background_task(
      priority, std::move(work), std::move(completion));
}

Result<TaskHandle> WindowRuntimeContext::try_spawn_background_task(
    TaskPriority priority,
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  return runtime.try_spawn_background_task(
      priority, std::move(work), std::move(completion));
}

} // namespace cgpui
