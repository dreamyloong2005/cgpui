#include "cgpui/ui/runtime.hpp"

#include <optional>
#include <utility>

namespace cgpui {

Subscription::~Subscription() {
  (void)release();
}

Subscription::Subscription(Subscription&& other) noexcept
    : runtime_(std::exchange(other.runtime_, nullptr)),
      id_(std::exchange(other.id_, {})) {}

Subscription& Subscription::operator=(Subscription&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  (void)release();
  runtime_ = std::exchange(other.runtime_, nullptr);
  id_ = std::exchange(other.id_, {});
  return *this;
}

bool Subscription::connected() const {
  return runtime_ != nullptr && runtime_->subscription_connected(id_);
}

bool Subscription::release() {
  if (runtime_ == nullptr) {
    return false;
  }

  WindowRuntime* runtime = runtime_;
  const SubscriptionId id = id_;
  runtime_ = nullptr;
  id_ = {};
  return runtime->remove_subscription(id);
}

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
