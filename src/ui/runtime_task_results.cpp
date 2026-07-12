#include "ui_internal.hpp"

#include <expected>
#include <utility>

namespace cgpui {

Result<TaskHandle> WindowRuntime::try_spawn_task(
    TaskCompletionCallback callback) {
  return try_spawn_task(TaskPriority::normal, std::move(callback));
}

Result<TaskHandle> WindowRuntime::try_spawn_background_task(
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) {
  return try_spawn_background_task(
      TaskPriority::normal, std::move(work), std::move(completion));
}

} // namespace cgpui
