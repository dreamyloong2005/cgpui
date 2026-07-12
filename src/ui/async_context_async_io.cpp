#include "ui_internal.hpp"

namespace cgpui {

AsyncIoHook AsyncContextCapability::create_async_io_hook(
    AsyncIoCompletionCallback callback) const {
  return context_->create_async_io_hook(std::move(callback));
}

AsyncIoHook AsyncContextCapability::create_async_io_hook(
    TaskPriority priority,
    AsyncIoCompletionCallback callback) const {
  return context_->create_async_io_hook(priority, std::move(callback));
}

} // namespace cgpui
