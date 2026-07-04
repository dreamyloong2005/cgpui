#include "ui_internal.hpp"

#include <algorithm>

namespace cgpui {

std::span<const EntitySubscription> WindowRuntime::subscriptions_for_view(
    ViewId view_id) const {
  subscription_query_buffer_.clear();
  for (const EntitySubscription& subscription : entity_subscriptions_) {
    if (subscription.view_id == view_id) {
      subscription_query_buffer_.push_back(subscription);
    }
  }
  return subscription_query_buffer_;
}

bool WindowRuntime::subscription_connected(SubscriptionId id) const {
  if (id.value == 0) {
    return false;
  }

  return std::ranges::any_of(
      entity_observers_,
      [id](const EntityObserver& observer) {
        return observer.subscription_id == id && observer.callback;
      });
}

bool WindowRuntime::remove_subscription(SubscriptionId id) {
  if (id.value == 0) {
    return false;
  }

  const auto previous_size = entity_observers_.size();
  entity_observers_.erase(
      std::ranges::remove_if(
          entity_observers_,
          [id](const EntityObserver& observer) {
            return observer.subscription_id == id;
          })
          .begin(),
      entity_observers_.end());
  return entity_observers_.size() != previous_size;
}

} // namespace cgpui
