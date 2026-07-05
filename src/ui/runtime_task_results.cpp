#include "ui_internal.hpp"

#include <expected>
#include <utility>

namespace cgpui {

Result<TaskHandle> WindowRuntime::try_spawn_task(
    TaskCompletionCallback callback) {
  if (!callback) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "foreground task completion callback is required",
    });
  }
  return spawn_task(std::move(callback));
}

Result<TaskHandle> WindowRuntime::try_spawn_background_task(
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) {
  if (!work) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "background task work is required",
    });
  }
  if (!completion) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "background task completion callback is required",
    });
  }
  return spawn_background_task(std::move(work), std::move(completion));
}

} // namespace cgpui
