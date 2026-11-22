#include "ui_internal.hpp"

namespace cgpui {

std::any& WindowRuntime::ensure_entity_store_value(std::type_index type, std::any (*factory)()) {
  if (const auto entry = state_->entities.entity_stores_.find(type); entry != state_->entities.entity_stores_.end()) {
    return entry->second;
  }
  return state_->entities.entity_stores_.try_emplace(type, factory()).first->second;
}

std::any* WindowRuntime::entity_store_value(std::type_index type) {
  const auto entry = state_->entities.entity_stores_.find(type);
  return entry == state_->entities.entity_stores_.end() ? nullptr : &entry->second;
}

const std::any* WindowRuntime::entity_store_value(std::type_index type) const {
  const auto entry = state_->entities.entity_stores_.find(type);
  return entry == state_->entities.entity_stores_.end() ? nullptr : &entry->second;
}

void WindowRuntime::did_insert_entity() { ++state_->entities.entity_count_; }
void WindowRuntime::did_remove_entity() {
  if (state_->entities.entity_count_ != 0) --state_->entities.entity_count_;
}

void WindowRuntime::subscribe_view_to_entity_value(
    ViewId view, std::type_index type, std::uint64_t id) {
  if (view.value == 0 || id == 0) return;
  for (const auto& subscription : state_->entities.entity_subscriptions_) {
    if (subscription.view_id == view && subscription.entity_type == type &&
        subscription.entity_id_value == id) return;
  }
  state_->entities.entity_subscriptions_.push_back(EntitySubscription{
      .view_id = view, .entity_type = type, .entity_id_value = id});
}

} // namespace cgpui
