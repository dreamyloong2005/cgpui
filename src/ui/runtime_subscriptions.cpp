#include "ui_internal.hpp"

#include <algorithm>

namespace cgpui {

std::span<const EntitySubscription> WindowRuntime::subscriptions_for_view(
    ViewId view_id) const {
  state_->entities.subscription_query_buffer_.clear();
  for (const EntitySubscription& subscription : state_->entities.entity_subscriptions_) {
    if (subscription.view_id == view_id) {
      state_->entities.subscription_query_buffer_.push_back(subscription);
    }
  }
  return state_->entities.subscription_query_buffer_;
}

bool WindowRuntime::subscription_connected(SubscriptionId id) const {
  if (id.value == 0) {
    return false;
  }

  return std::ranges::any_of(
             state_->entities.entity_observers_,
             [id](const EntityObserver& observer) {
               return observer.subscription_id == id && observer.callback;
             }) ||
         std::ranges::any_of(
             state_->entities.window_observers_,
             [id](const WindowObserver& observer) {
               return observer.subscription_id == id && observer.callback;
             }) ||
         std::ranges::any_of(
             state_->entities.view_observers_,
             [id](const ViewObserver& observer) {
               return observer.subscription_id == id && observer.callback;
             });
}

bool WindowRuntime::remove_subscription(SubscriptionId id) {
  if (id.value == 0) {
    return false;
  }

  const auto previous_entity_size = state_->entities.entity_observers_.size();
  state_->entities.entity_observers_.erase(
      std::ranges::remove_if(
          state_->entities.entity_observers_,
          [id](const EntityObserver& observer) {
            return observer.subscription_id == id;
          })
          .begin(),
      state_->entities.entity_observers_.end());

  const auto previous_window_size = state_->entities.window_observers_.size();
  state_->entities.window_observers_.erase(
      std::ranges::remove_if(
          state_->entities.window_observers_,
          [id](const WindowObserver& observer) {
            return observer.subscription_id == id;
          })
          .begin(),
      state_->entities.window_observers_.end());

  const auto previous_view_size = state_->entities.view_observers_.size();
  state_->entities.view_observers_.erase(
      std::ranges::remove_if(
          state_->entities.view_observers_,
          [id](const ViewObserver& observer) {
            return observer.subscription_id == id;
          })
          .begin(),
      state_->entities.view_observers_.end());

  return state_->entities.entity_observers_.size() != previous_entity_size ||
         state_->entities.window_observers_.size() != previous_window_size ||
         state_->entities.view_observers_.size() != previous_view_size;
}

} // namespace cgpui
