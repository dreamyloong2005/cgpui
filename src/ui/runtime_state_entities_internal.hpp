#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeEntityState {
  std::unordered_map<std::type_index, std::any> globals_;
  std::unordered_map<std::type_index, std::any> entity_stores_;
  std::size_t entity_count_ = 0;
  std::vector<EntitySubscription> entity_subscriptions_;
  std::vector<EntityObserver> entity_observers_;
  std::vector<WindowObserver> window_observers_;
  std::vector<ViewObserver> view_observers_;
  std::uint64_t next_subscription_id_ = 1;
  mutable std::vector<EntitySubscription> subscription_query_buffer_;
};

} // namespace cgpui
