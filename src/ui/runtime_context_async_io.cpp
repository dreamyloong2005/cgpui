#include "ui_internal.hpp"

namespace cgpui {

AsyncIoHook WindowRuntimeContext::create_async_io_hook(
    AsyncIoCompletionCallback callback) const {
  return create_async_io_hook(TaskPriority::normal, std::move(callback));
}

AsyncIoHook WindowRuntimeContext::create_async_io_hook(
    TaskPriority priority,
    AsyncIoCompletionCallback callback) const {
  return runtime.create_async_io_hook(priority, runtime.bind_window_io_callback(window_runtime_id, std::move(callback)));
}

} // namespace cgpui
