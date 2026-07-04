#include "cgpui/ui/runtime.hpp"

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

} // namespace cgpui
