#include "ui_internal.hpp"

namespace cgpui {

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
  for (const EntityObserver& observer : entity_observers_) {
    if (observer.entity_type == entity_type &&
        observer.entity_id_value == entity_id_value &&
        observer.callback) {
      notified = true;
      observer.callback(context(), entity_id_value);
    }
  }
  if (notified) {
    request_render();
  }
  return notified;
}

} // namespace cgpui
