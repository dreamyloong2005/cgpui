#include "ui_internal.hpp"

namespace cgpui {

SubscriptionId WindowRuntime::append_entity_observer(std::type_index type, std::uint64_t entity_id,
    std::function<void(const WindowRuntimeContext&, std::uint64_t)> callback, bool tracked) {
  const SubscriptionId subscription = tracked ? SubscriptionId{state_->entities.next_subscription_id_++} : SubscriptionId{};
  state_->entities.entity_observers_.push_back(EntityObserver{.subscription_id = subscription,
      .entity_type = type, .entity_id_value = entity_id, .callback = std::move(callback)});
  return subscription;
}

SubscriptionId WindowRuntime::append_window_observer(WindowObservationCallback callback, bool tracked) {
  const SubscriptionId id = tracked ? SubscriptionId{state_->entities.next_subscription_id_++} : SubscriptionId{};
  state_->entities.window_observers_.push_back(WindowObserver{
      .subscription_id = id, .runtime_id = state_->windows.root_window_runtime_id_, .callback = std::move(callback)});
  return id;
}

SubscriptionId WindowRuntime::append_view_observer(
    ViewId view, ViewObservationCallback callback, bool tracked) {
  const SubscriptionId id = tracked ? SubscriptionId{state_->entities.next_subscription_id_++} : SubscriptionId{};
  state_->entities.view_observers_.push_back(ViewObserver{
      .subscription_id = id, .view_id = view, .callback = std::move(callback)});
  return id;
}

} // namespace cgpui
