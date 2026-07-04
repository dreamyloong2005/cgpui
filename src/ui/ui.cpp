#include "cgpui/ui/runtime.hpp"

#include <optional>

namespace cgpui {

bool TaskHandle::active() const {
  return runtime_ != nullptr && runtime_->task_active(id_);
}

bool TaskHandle::complete() const {
  return runtime_ != nullptr && runtime_->task_complete(id_);
}

bool TaskHandle::cancelled() const {
  return runtime_ != nullptr && runtime_->task_cancelled(id_);
}

bool TaskHandle::cancel() {
  return runtime_ != nullptr && runtime_->cancel_task(id_);
}

bool TaskCancellationToken::cancellation_requested() const {
  return state_ != nullptr && state_->load();
}

bool AnimationHandle::active() const {
  return runtime_ != nullptr && runtime_->animation_active(id_);
}

bool AnimationHandle::complete() const {
  return runtime_ != nullptr && runtime_->animation_complete(id_);
}

std::optional<AnimationSnapshot> AnimationHandle::progress() const {
  if (runtime_ == nullptr) {
    return std::nullopt;
  }
  return runtime_->animation_snapshot(id_);
}

bool AnimationHandle::cancel() {
  return runtime_ != nullptr && runtime_->cancel_animation(id_);
}


} // namespace cgpui
