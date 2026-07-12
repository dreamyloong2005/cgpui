#include "ui_internal.hpp"
#include "runtime_async_io_internal.hpp"

namespace cgpui {

bool AsyncIoHook::active() const {
  return state_ != nullptr && state_->status.load() ==
      detail::AsyncIoHookStatus::active;
}

bool AsyncIoHook::complete() const {
  return state_ != nullptr && state_->status.load() ==
      detail::AsyncIoHookStatus::complete;
}

bool AsyncIoHook::cancelled() const {
  return state_ != nullptr && state_->status.load() ==
      detail::AsyncIoHookStatus::cancelled;
}

bool AsyncIoHook::notify(AsyncIoPayload payload) const {
  if (state_ == nullptr) return false;
  std::lock_guard lock(state_->mutex);
  return state_->runtime != nullptr &&
      state_->runtime->notify_async_io(id_, AsyncIoResult{std::move(payload)});
}

bool AsyncIoHook::notify_error(Error error) const {
  if (state_ == nullptr) return false;
  std::lock_guard lock(state_->mutex);
  return state_->runtime != nullptr && state_->runtime->notify_async_io(
      id_, std::unexpected(std::move(error)));
}

bool AsyncIoHook::cancel() const {
  if (state_ == nullptr) return false;
  std::lock_guard lock(state_->mutex);
  return state_->runtime != nullptr && state_->runtime->cancel_async_io(id_);
}

} // namespace cgpui
