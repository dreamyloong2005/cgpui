#include "ui_internal.hpp"

namespace cgpui {

namespace {

struct PendingEntityObserver {
  SubscriptionId subscription_id;
  std::function<void(const WindowRuntimeContext&, std::uint64_t)> callback;
};

} // namespace

bool WindowRuntime::notify_entity_changed(
    std::type_index entity_type,
    std::uint64_t entity_id_value) {
  bool notified = false;
  for (const EntitySubscription& subscription : entity_subscriptions_) {
    if (subscription.entity_type == entity_type &&
        subscription.entity_id_value == entity_id_value) {
      notified = true;
    }
  }
  std::vector<PendingEntityObserver> pending_observers;
  for (const EntityObserver& observer : entity_observers_) {
    if (observer.entity_type == entity_type &&
        observer.entity_id_value == entity_id_value &&
        observer.callback) {
      pending_observers.push_back(PendingEntityObserver{
          .subscription_id = observer.subscription_id,
          .callback = observer.callback,
      });
    }
  }
  for (const PendingEntityObserver& pending : pending_observers) {
    auto callback = pending.callback;
    if (pending.subscription_id.value != 0) {
      callback = {};
      for (const EntityObserver& observer : entity_observers_) {
        if (observer.subscription_id == pending.subscription_id &&
            observer.callback) {
          callback = observer.callback;
          break;
        }
      }
    }
    if (callback) {
      notified = true;
      callback(context(), entity_id_value);
    }
  }
  if (notified) {
    request_render();
  }
  return notified;
}

} // namespace cgpui
