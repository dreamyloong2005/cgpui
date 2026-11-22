#include "ui_internal.hpp"
#include "runtime_async_io_internal.hpp"

namespace cgpui {

AsyncIoHook WindowRuntime::RuntimeAsyncIoRegistry::create(
    WindowRuntime& runtime,
    TaskPriority priority,
    AsyncIoCompletionCallback callback) {
  std::lock_guard lock(mutex_);
  if (closed_) return {};
  const AsyncIoId id{next_id_++};
  auto state = std::make_shared<detail::AsyncIoHookState>();
  state->runtime = &runtime;
  state->id = id;
  records_.push_back(Record{
      .id = id,
      .priority = priority,
      .callback = std::move(callback),
      .state = state,
  });
  return AsyncIoHook(id, std::move(state));
}

bool WindowRuntime::RuntimeAsyncIoRegistry::notify(
    AsyncIoId id,
    AsyncIoResult result) {
  std::lock_guard lock(mutex_);
  const auto record = std::find_if(
      records_.begin(), records_.end(),
      [id](const Record& value) { return value.id == id; });
  if (record == records_.end() || record->state->status.load() !=
      detail::AsyncIoHookStatus::active) {
    return false;
  }
  record->state->status.store(detail::AsyncIoHookStatus::queued);
  completions_.push_back(Completion{
      .id = id,
      .priority = record->priority,
      .result = std::move(result),
  });
  return true;
}

bool WindowRuntime::RuntimeAsyncIoRegistry::cancel(
    AsyncIoId id, AsyncIoCompletionCallback& retired_callback) {
  std::lock_guard lock(mutex_);
  const auto record = std::find_if(
      records_.begin(), records_.end(),
      [id](const Record& value) { return value.id == id; });
  if (record == records_.end()) return false;
  const auto status = record->state->status.load();
  if (status == detail::AsyncIoHookStatus::complete ||
      status == detail::AsyncIoHookStatus::cancelled) {
    return false;
  }
  record->state->status.store(detail::AsyncIoHookStatus::cancelled);
  retired_callback = std::move(record->callback);
  std::erase_if(
      completions_, [id](const Completion& value) { return value.id == id; });
  return true;
}

std::vector<WindowRuntime::RuntimeAsyncIoRegistry::Completion>
WindowRuntime::RuntimeAsyncIoRegistry::take_completions() {
  std::lock_guard lock(mutex_);
  std::vector<Completion> completions;
  completions.swap(completions_);
  return completions;
}

AsyncIoCompletionCallback WindowRuntime::RuntimeAsyncIoRegistry::complete(
    AsyncIoId id) {
  std::lock_guard lock(mutex_);
  const auto record = std::find_if(
      records_.begin(), records_.end(),
      [id](const Record& value) { return value.id == id; });
  if (record == records_.end() || record->state->status.load() !=
      detail::AsyncIoHookStatus::queued) {
    return {};
  }
  record->state->status.store(detail::AsyncIoHookStatus::complete);
  return std::move(record->callback);
}

void WindowRuntime::RuntimeAsyncIoRegistry::shutdown() {
  std::vector<Record> retired;
  {
    std::lock_guard lock(mutex_);
    closed_ = true;
    retired.swap(records_);
    completions_.clear();
  }
  for (const auto& record : retired) {
    const auto& state = record.state;
    std::lock_guard lock(state->mutex);
    state->runtime = nullptr;
    if (state->status.load() != detail::AsyncIoHookStatus::complete) {
      state->status.store(detail::AsyncIoHookStatus::cancelled);
    }
  }
}

AsyncIoHook WindowRuntime::create_async_io_hook(
    AsyncIoCompletionCallback callback) {
  return create_async_io_hook(TaskPriority::normal, std::move(callback));
}

AsyncIoHook WindowRuntime::create_async_io_hook(
    TaskPriority priority,
    AsyncIoCompletionCallback callback) {
  return state_->tasks.async_io_registry_->create(*this, priority, std::move(callback));
}

bool WindowRuntime::notify_async_io(AsyncIoId id, AsyncIoResult result) {
  if (!state_->tasks.async_io_registry_->notify(id, std::move(result))) return false;
  request_platform_wakeup();
  return true;
}

bool WindowRuntime::cancel_async_io(
    AsyncIoId id, AsyncIoCompletionCallback& retired_callback) {
  return state_->tasks.async_io_registry_->cancel(id, retired_callback);
}

} // namespace cgpui
