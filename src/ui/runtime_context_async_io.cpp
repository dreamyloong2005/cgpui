#include "ui_internal.hpp"

namespace cgpui {

AsyncIoHook WindowRuntimeContext::create_async_io_hook(
    AsyncIoCompletionCallback callback) const {
  return runtime.create_async_io_hook(std::move(callback));
}

AsyncIoHook WindowRuntimeContext::create_async_io_hook(
    TaskPriority priority,
    AsyncIoCompletionCallback callback) const {
  return runtime.create_async_io_hook(priority, std::move(callback));
}

} // namespace cgpui
