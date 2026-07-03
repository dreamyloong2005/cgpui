#pragma once

#include "cgpui/ui/window_runtime.hpp"

#include <any>
#include <cstdint>
#include <optional>
#include <typeindex>
#include <typeinfo>
#include <utility>

namespace cgpui {

template <typename T>
void AppContext::set_global(T global_value) const {
  runtime.set_global<T>(std::move(global_value));
}

template <typename T>
const T* AppContext::global() const {
  return runtime.global<T>();
}

template <typename T, typename Update>
bool AppContext::update_global(Update&& update) const {
  return runtime.update_global<T>(std::forward<Update>(update));
}

template <typename T>
void WindowRuntimeContext::set_global(T global_value) const {
  runtime.set_global<T>(std::move(global_value));
}

template <typename T>
const T* WindowRuntimeContext::global() const {
  return runtime.global<T>();
}

template <typename T, typename Update>
bool WindowRuntimeContext::update_global(Update&& update) const {
  return runtime.update_global<T>(std::forward<Update>(update));
}

template <typename T>
T* WindowRuntimeContext::element_state(ElementId element_id) const {
  return runtime.element_state<T>(element_id);
}

template <typename T, typename... Args>
T* WindowRuntimeContext::emplace_element_state(
    ElementId element_id,
    Args&&... args) const {
  return runtime.emplace_element_state<T>(
      element_id,
      std::forward<Args>(args)...);
}

template <typename T, typename... Args>
T* WindowRuntimeContext::element_state_or_init(
    ElementId element_id,
    Args&&... args) const {
  return runtime.element_state_or_init<T>(
      element_id,
      std::forward<Args>(args)...);
}

template <typename T>
EntityId<T> WindowRuntimeContext::insert_entity(T entity) const {
  return runtime.insert_entity<T>(std::move(entity));
}

template <typename T, typename... Args>
EntityId<T> WindowRuntimeContext::emplace_entity(Args&&... args) const {
  return runtime.emplace_entity<T>(std::forward<Args>(args)...);
}

template <typename T>
const T* WindowRuntimeContext::read_entity(EntityId<T> id) const {
  return runtime.read_entity(id);
}

template <typename T>
T* WindowRuntimeContext::mutate_entity(EntityId<T> id) const {
  return runtime.mutate_entity(id);
}

template <typename T>
bool WindowRuntimeContext::remove_entity(EntityId<T> id) const {
  return runtime.remove_entity(id);
}

template <typename T, typename... Args>
Model<T> WindowRuntimeContext::new_model(Args&&... args) const {
  return runtime.emplace_entity<T>(std::forward<Args>(args)...);
}

template <typename T>
const T* WindowRuntimeContext::read_model(Model<T> model) const {
  return runtime.read_entity(model);
}

template <typename T>
std::optional<Model<T>> WindowRuntimeContext::upgrade_entity(
    WeakEntity<T> entity) const {
  return runtime.upgrade_entity(entity);
}

template <typename T, typename Update>
bool WindowRuntimeContext::update_model(Model<T> model, Update&& update) const {
  T* stored_model = runtime.mutate_entity(model);
  if (stored_model == nullptr) {
    return false;
  }
  std::forward<Update>(update)(*stored_model);
  return runtime.notify_entity_changed(model);
}

template <typename T, typename Observer>
bool WindowRuntimeContext::observe_model(
    Model<T> model,
    Observer&& observer) const {
  return runtime.observe_model(model, std::forward<Observer>(observer));
}

template <typename T, typename Observer>
Subscription WindowRuntimeContext::observe_model_subscription(
    Model<T> model,
    Observer&& observer) const {
  return runtime.observe_model_subscription(
      model,
      std::forward<Observer>(observer));
}

template <typename T>
bool WindowRuntimeContext::remove_model(Model<T> model) const {
  return runtime.remove_entity(model);
}

template <typename T>
void WindowRuntimeContext::subscribe_view_to_entity(
    ViewId view_id,
    EntityId<T> entity_id) const {
  runtime.subscribe_view_to_entity(view_id, entity_id);
}

template <typename T>
EntityId<T> WindowRuntime::insert_entity(T entity) {
  const EntityId<T> id = entity_store<T>().insert(std::move(entity));
  entity_count_ += 1;
  return id;
}

template <typename T, typename... Args>
EntityId<T> WindowRuntime::emplace_entity(Args&&... args) {
  const EntityId<T> id =
      entity_store<T>().emplace(std::forward<Args>(args)...);
  entity_count_ += 1;
  return id;
}

template <typename T>
const T* WindowRuntime::read_entity(EntityId<T> id) const {
  const EntityStore<T>* store = find_entity_store<T>();
  if (store == nullptr) {
    return nullptr;
  }
  return store->get(id);
}

template <typename T>
std::optional<EntityId<T>> WindowRuntime::upgrade_entity(
    WeakEntity<T> entity) const {
  if (entity.empty() || read_entity(entity.id()) == nullptr) {
    return std::nullopt;
  }
  return entity.id();
}

template <typename T>
T* WindowRuntime::mutate_entity(EntityId<T> id) {
  EntityStore<T>* store = find_entity_store<T>();
  if (store == nullptr) {
    return nullptr;
  }
  return store->get(id);
}

template <typename T, typename Observer>
bool WindowRuntime::observe_model(
    EntityId<T> entity_id,
    Observer&& observer) {
  ModelObserver<T> observer_fn{std::forward<Observer>(observer)};
  if (!observer_fn || read_entity(entity_id) == nullptr) {
    return false;
  }

  entity_observers_.push_back(EntityObserver{
      .subscription_id = {},
      .entity_type = std::type_index(typeid(T)),
      .entity_id_value = entity_id.value,
      .callback =
          [observer = std::move(observer_fn)](
              const WindowRuntimeContext& context,
              std::uint64_t entity_id_value) {
            observer(context, Model<T>{entity_id_value});
          },
  });
  return true;
}

template <typename T, typename Observer>
Subscription WindowRuntime::observe_model_subscription(
    EntityId<T> entity_id,
    Observer&& observer) {
  ModelObserver<T> observer_fn{std::forward<Observer>(observer)};
  if (!observer_fn || read_entity(entity_id) == nullptr) {
    return {};
  }

  const SubscriptionId subscription_id{next_subscription_id_++};
  entity_observers_.push_back(EntityObserver{
      .subscription_id = subscription_id,
      .entity_type = std::type_index(typeid(T)),
      .entity_id_value = entity_id.value,
      .callback =
          [observer = std::move(observer_fn)](
              const WindowRuntimeContext& context,
              std::uint64_t entity_id_value) {
            observer(context, Model<T>{entity_id_value});
          },
  });
  return Subscription(*this, subscription_id);
}

template <typename T>
bool WindowRuntime::remove_entity(EntityId<T> id) {
  EntityStore<T>* store = find_entity_store<T>();
  if (store == nullptr) {
    return false;
  }
  const bool removed = store->remove(id);
  if (removed) {
    if (entity_count_ > 0) {
      entity_count_ -= 1;
    }
    (void)notify_entity_changed(id);
  }
  return removed;
}

template <typename T>
void WindowRuntime::subscribe_view_to_entity(
    ViewId view_id,
    EntityId<T> entity_id) {
  if (view_id.value == 0 || entity_id.value == 0) {
    return;
  }

  const std::type_index entity_type(typeid(T));
  for (const EntitySubscription& subscription : entity_subscriptions_) {
    if (subscription.view_id == view_id &&
        subscription.entity_type == entity_type &&
        subscription.entity_id_value == entity_id.value) {
      return;
    }
  }

  entity_subscriptions_.push_back(EntitySubscription{
      .view_id = view_id,
      .entity_type = entity_type,
      .entity_id_value = entity_id.value,
  });
}

template <typename T>
bool WindowRuntime::notify_entity_changed(EntityId<T> entity_id) {
  return notify_entity_changed(std::type_index(typeid(T)), entity_id.value);
}

template <typename T>
void WindowRuntime::set_global(T global_value) {
  globals_[std::type_index(typeid(T))] = std::move(global_value);
  request_render();
}

template <typename T>
const T* WindowRuntime::global() const {
  const auto entry = globals_.find(std::type_index(typeid(T)));
  if (entry == globals_.end()) {
    return nullptr;
  }
  return &std::any_cast<const T&>(entry->second);
}

template <typename T>
T* WindowRuntime::global() {
  auto entry = globals_.find(std::type_index(typeid(T)));
  if (entry == globals_.end()) {
    return nullptr;
  }
  return &std::any_cast<T&>(entry->second);
}

template <typename T, typename Update>
bool WindowRuntime::update_global(Update&& update) {
  T* stored_global = global<T>();
  if (stored_global == nullptr) {
    return false;
  }
  std::forward<Update>(update)(*stored_global);
  request_render();
  return true;
}

template <typename T>
T* WindowRuntime::element_state(ElementId element_id) {
  if (owned_element_tree_ == nullptr) {
    return nullptr;
  }
  return owned_element_tree_->state<T>(element_id);
}

template <typename T>
const T* WindowRuntime::element_state(ElementId element_id) const {
  if (owned_element_tree_ == nullptr) {
    return nullptr;
  }
  return owned_element_tree_->state<T>(element_id);
}

template <typename T, typename... Args>
T* WindowRuntime::emplace_element_state(ElementId element_id, Args&&... args) {
  if (owned_element_tree_ == nullptr) {
    return nullptr;
  }
  return owned_element_tree_->emplace_state<T>(
      element_id,
      std::forward<Args>(args)...);
}

template <typename T, typename... Args>
T* WindowRuntime::element_state_or_init(
    ElementId element_id,
    Args&&... args) {
  if (owned_element_tree_ == nullptr) {
    return nullptr;
  }
  return owned_element_tree_->state_or_init<T>(
      element_id,
      std::forward<Args>(args)...);
}

template <typename T>
EntityStore<T>& WindowRuntime::entity_store() {
  auto [entry, inserted] = entity_stores_.try_emplace(
      std::type_index(typeid(T)),
      EntityStore<T>{});
  (void)inserted;
  return std::any_cast<EntityStore<T>&>(entry->second);
}

template <typename T>
const EntityStore<T>* WindowRuntime::find_entity_store() const {
  const auto entry = entity_stores_.find(std::type_index(typeid(T)));
  if (entry == entity_stores_.end()) {
    return nullptr;
  }
  return &std::any_cast<const EntityStore<T>&>(entry->second);
}

template <typename T>
EntityStore<T>* WindowRuntime::find_entity_store() {
  auto entry = entity_stores_.find(std::type_index(typeid(T)));
  if (entry == entity_stores_.end()) {
    return nullptr;
  }
  return &std::any_cast<EntityStore<T>&>(entry->second);
}



} // namespace cgpui
