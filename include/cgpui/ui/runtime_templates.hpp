#pragma once

#include "cgpui/ui/window_runtime.hpp"

#include <any>
#include <cstdint>
#include <functional>
#include <optional>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <utility>

namespace cgpui {

namespace detail {

template <typename Update, typename T>
decltype(auto) invoke_entity_update(
    Update&& update,
    T& entity,
    const WindowRuntimeContext& context) {
  if constexpr (std::is_invocable_v<Update, T&, const WindowRuntimeContext&>) {
    return std::invoke(
        std::forward<Update>(update),
        entity,
        context);
  } else {
    return std::invoke(std::forward<Update>(update), entity);
  }
}

template <typename Update, typename T>
using EntityUpdateResult = decltype(invoke_entity_update(
    std::declval<Update>(),
    std::declval<T&>(),
    std::declval<const WindowRuntimeContext&>()));

template <typename Result>
struct EntityUpdateReturn {
  using type = std::optional<std::remove_cvref_t<Result>>;
};

template <>
struct EntityUpdateReturn<void> {
  using type = bool;
};

template <typename Result>
using EntityUpdateReturnT = typename EntityUpdateReturn<Result>::type;

inline std::uintptr_t entity_context_token(
    const WindowRuntimeContext& context) {
  return reinterpret_cast<std::uintptr_t>(&context.runtime);
}

inline std::uintptr_t view_context_token(
    const WindowRuntimeContext& context) {
  return reinterpret_cast<std::uintptr_t>(&context.runtime);
}

template <typename Observer, typename ObserverT, typename ObservedT>
void invoke_entity_to_entity_observer(
    Observer& observer,
    ObserverT& observer_entity,
    EntityHandle<ObservedT> observed_entity,
    const WindowRuntimeContext& context) {
  if constexpr (std::is_invocable_v<
                    Observer&,
                    ObserverT&,
                    EntityHandle<ObservedT>,
                    const WindowRuntimeContext&>) {
    std::invoke(observer, observer_entity, observed_entity, context);
  } else if constexpr (std::is_invocable_v<
                           Observer&,
                           ObserverT&,
                           const WindowRuntimeContext&,
                           EntityHandle<ObservedT>>) {
    std::invoke(observer, observer_entity, context, observed_entity);
  } else {
    std::invoke(observer, observer_entity, observed_entity);
  }
}

} // namespace detail

template <typename T>
CrossThreadEntity<T> AsyncContextCapability::entity(
    EntityHandle<T> entity) const {
  if (context_ == nullptr || entity.empty() ||
      !entity.matches_context(detail::entity_context_token(*context_)) ||
      context_->read_entity(entity) == nullptr) {
    return {};
  }
  return CrossThreadEntity<T>(
      entity, context_->runtime.cross_thread_entity_queue_);
}

template <typename T>
bool CrossThreadEntity<T>::read(
    ReadCallback callback,
    CrossThreadEntityAccessCompletion completion) const {
  if (empty() || !callback) return false;
  return detail::enqueue_cross_thread_entity_operation(
      queue_,
      [entity = entity_, callback = std::move(callback),
       completion = std::move(completion)](
          const WindowRuntimeContext& context) mutable {
        CrossThreadEntityAccessStatus status =
            CrossThreadEntityAccessStatus::context_mismatch;
        if (entity.matches_context(detail::entity_context_token(context))) {
          const T* value = context.read_entity(entity);
          status = value == nullptr ? CrossThreadEntityAccessStatus::missing
                                    : CrossThreadEntityAccessStatus::read;
          if (value != nullptr) callback(*value, context);
        }
        if (completion) completion(status, context);
      });
}

template <typename T>
bool CrossThreadEntity<T>::update(
    UpdateCallback callback,
    CrossThreadEntityAccessCompletion completion) const {
  if (empty() || !callback) return false;
  return detail::enqueue_cross_thread_entity_operation(
      queue_,
      [entity = entity_, callback = std::move(callback),
       completion = std::move(completion)](
          const WindowRuntimeContext& context) mutable {
        CrossThreadEntityAccessStatus status =
            CrossThreadEntityAccessStatus::context_mismatch;
        if (entity.matches_context(detail::entity_context_token(context))) {
          const bool updated = context.update_entity(
              entity,
              [&](T& value, const WindowRuntimeContext&) {
                callback(value, context);
              });
          status = updated ? CrossThreadEntityAccessStatus::updated
                           : CrossThreadEntityAccessStatus::missing;
        }
        if (completion) completion(status, context);
      });
}

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
T* ElementContextCapability::state() const {
  return context_->template element_state<T>(element_id_);
}

template <typename T, typename... Args>
T* ElementContextCapability::emplace_state(Args&&... args) const {
  return context_->template emplace_element_state<T>(
      element_id_,
      std::forward<Args>(args)...);
}

template <typename T, typename... Args>
T* ElementContextCapability::state_or_init(Args&&... args) const {
  return context_->template element_state_or_init<T>(
      element_id_,
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
const T* WindowRuntimeContext::read_entity(EntityHandle<T> entity) const {
  if (entity.empty() ||
      !entity.matches_context(detail::entity_context_token(*this))) {
    return nullptr;
  }

  return runtime.read_entity(entity.id());
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

template <typename T, typename... Args>
EntityHandle<T> WindowRuntimeContext::new_entity(Args&&... args) const {
  return EntityHandle<T>(
      runtime.emplace_entity<T>(std::forward<Args>(args)...),
      detail::entity_context_token(*this));
}

template <typename T>
EntityHandle<T> WindowRuntimeContext::insert_entity_handle(T entity) const {
  return EntityHandle<T>(
      runtime.insert_entity<T>(std::move(entity)),
      detail::entity_context_token(*this));
}

template <typename T>
const T* WindowRuntimeContext::read_model(Model<T> model) const {
  return runtime.read_entity(model);
}

template <typename T>
ViewId ViewContextCapability<T>::view_id() const {
  return context_->view_id;
}

template <typename T>
ViewHandle<T> ViewContextCapability<T>::view() const {
  return context_->template view<T>();
}

template <typename T>
WeakViewHandle<T> ViewContextCapability<T>::weak_view() const {
  return context_->template weak_view<T>();
}

template <typename T>
std::optional<ViewHandle<T>> ViewContextCapability<T>::upgrade(
    WeakViewHandle<T> view) const {
  return context_->template upgrade_view<T>(view);
}

template <typename T>
const T* ViewContextCapability<T>::read(ViewHandle<T> view) const {
  return context_->template read_view<T>(view);
}

template <typename T>
const T* ViewContextCapability<T>::current() const {
  return read(view());
}

template <typename T>
template <typename Observer>
bool ViewContextCapability<T>::observe(
    ViewHandle<T> view,
    Observer&& observer) const {
  return context_ != nullptr &&
         context_->observe_view(view, std::forward<Observer>(observer));
}

template <typename T>
template <typename Observer>
Subscription ViewContextCapability<T>::observe_subscription(
    ViewHandle<T> view,
    Observer&& observer) const {
  if (context_ == nullptr) {
    return {};
  }
  return context_->observe_view_subscription(
      view,
      std::forward<Observer>(observer));
}

template <typename Observer>
bool WindowContextCapability::observe(Observer&& observer) const {
  return context_ != nullptr &&
         context_->observe_window(std::forward<Observer>(observer));
}

template <typename Observer>
Subscription WindowContextCapability::observe_subscription(
    Observer&& observer) const {
  if (context_ == nullptr) {
    return {};
  }
  return context_->observe_window_subscription(
      std::forward<Observer>(observer));
}

template <typename T>
ViewContextCapability<T> WindowRuntimeContext::view_context() const {
  return ViewContextCapability<T>(*this);
}

template <typename T>
ViewHandle<T> WindowRuntimeContext::view() const {
  return ViewHandle<T>(view_id, detail::view_context_token(*this));
}

template <typename T>
WeakViewHandle<T> WindowRuntimeContext::weak_view() const {
  return WeakViewHandle<T>(view_id, detail::view_context_token(*this));
}

template <typename T>
std::optional<ViewHandle<T>> WindowRuntimeContext::upgrade_view(
    WeakViewHandle<T> view) const {
  if (!view.matches_context(detail::view_context_token(*this))) {
    return std::nullopt;
  }
  const std::optional<ViewId> upgraded = runtime.upgrade_view(view.untyped());
  if (!upgraded.has_value()) {
    return std::nullopt;
  }
  return ViewHandle<T>(*upgraded, view.context_token());
}

template <typename T>
const T* WindowRuntimeContext::read_view(ViewHandle<T> view) const {
  if (view.empty() ||
      !view.matches_context(detail::view_context_token(*this))) {
    return nullptr;
  }
  return dynamic_cast<const T*>(runtime.find_view(view.id()));
}

template <typename Observer>
bool WindowRuntimeContext::observe_window(Observer&& observer) const {
  return runtime.observe_window(
      [observer = std::forward<Observer>(observer)](
          const WindowRuntimeContext& context) mutable {
        observer(context, context.window_context());
      });
}

template <typename Observer>
Subscription WindowRuntimeContext::observe_window_subscription(
    Observer&& observer) const {
  return runtime.observe_window_subscription(
      [observer = std::forward<Observer>(observer)](
          const WindowRuntimeContext& context) mutable {
        observer(context, context.window_context());
      });
}

template <typename T, typename Observer>
bool WindowRuntimeContext::observe_view(
    ViewHandle<T> view,
    Observer&& observer) const {
  if (view.empty() || read_view(view) == nullptr) {
    return false;
  }

  return runtime.observe_view(
      view.id(),
      [observer = std::forward<Observer>(observer)](
          const WindowRuntimeContext& context,
          ViewId view_id) mutable {
        observer(
            context,
            ViewHandle<T>(view_id, detail::view_context_token(context)));
      });
}

template <typename T, typename Observer>
Subscription WindowRuntimeContext::observe_view_subscription(
    ViewHandle<T> view,
    Observer&& observer) const {
  if (view.empty() || read_view(view) == nullptr) {
    return {};
  }

  return runtime.observe_view_subscription(
      view.id(),
      [observer = std::forward<Observer>(observer)](
          const WindowRuntimeContext& context,
          ViewId view_id) mutable {
        observer(
            context,
            ViewHandle<T>(view_id, detail::view_context_token(context)));
      });
}

template <typename T>
template <typename Context, typename Observer>
bool ViewHandle<T>::observe(
    const Context& context,
    Observer&& observer) const {
  return context.observe_view(*this, std::forward<Observer>(observer));
}

template <typename T>
template <typename Context, typename Observer>
Subscription ViewHandle<T>::observe_subscription(
    const Context& context,
    Observer&& observer) const {
  return context.observe_view_subscription(
      *this,
      std::forward<Observer>(observer));
}

template <typename T>
EntityHandle<T> WindowRuntimeContext::entity(EntityId<T> id) const {
  return EntityHandle<T>(id, detail::entity_context_token(*this));
}

template <typename T>
WeakEntity<T> WindowRuntimeContext::weak_entity(EntityId<T> id) const {
  return WeakEntity<T>(id, detail::entity_context_token(*this));
}

template <typename T>
std::optional<Model<T>> WindowRuntimeContext::upgrade_entity(
    WeakEntity<T> entity) const {
  if (!entity.matches_context(detail::entity_context_token(*this))) {
    return std::nullopt;
  }

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

template <typename T, typename Update>
auto WindowRuntimeContext::update_entity(
    EntityHandle<T> entity,
    Update&& update) const {
  using Result = detail::EntityUpdateResult<Update, T>;
  using Return = detail::EntityUpdateReturnT<Result>;

  if (entity.empty() ||
      !entity.matches_context(detail::entity_context_token(*this))) {
    if constexpr (std::is_void_v<Result>) {
      return false;
    } else {
      return Return{};
    }
  }

  T* stored_entity = runtime.mutate_entity(entity.id());
  if (stored_entity == nullptr) {
    if constexpr (std::is_void_v<Result>) {
      return false;
    } else {
      return Return{};
    }
  }

  if constexpr (std::is_void_v<Result>) {
    detail::invoke_entity_update(
        std::forward<Update>(update),
        *stored_entity,
        *this);
    (void)runtime.notify_entity_changed(entity.id());
    return true;
  } else {
    auto result = detail::invoke_entity_update(
        std::forward<Update>(update),
        *stored_entity,
        *this);
    (void)runtime.notify_entity_changed(entity.id());
    return Return{std::move(result)};
  }
}

template <typename T>
bool WindowRuntimeContext::invalidate_entity(EntityHandle<T> entity) const {
  if (entity.empty() ||
      !entity.matches_context(detail::entity_context_token(*this))) {
    return false;
  }

  return runtime.invalidate_entity(entity.id());
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

template <typename T, typename Observer>
bool WindowRuntimeContext::observe_entity(
    EntityHandle<T> entity,
    Observer&& observer) const {
  if (entity.empty() ||
      !entity.matches_context(detail::entity_context_token(*this))) {
    return false;
  }

  return observe_model(
      entity.id(),
      [observer = std::forward<Observer>(observer)](
          const WindowRuntimeContext& context,
          Model<T> model) mutable {
        observer(context, context.entity(model));
      });
}

template <typename T, typename Observer>
Subscription WindowRuntimeContext::observe_entity_subscription(
    EntityHandle<T> entity,
    Observer&& observer) const {
  if (entity.empty() ||
      !entity.matches_context(detail::entity_context_token(*this))) {
    return {};
  }

  return observe_model_subscription(
      entity.id(),
      [observer = std::forward<Observer>(observer)](
          const WindowRuntimeContext& context,
          Model<T> model) mutable {
        observer(context, context.entity(model));
      });
}

template <typename ObserverT, typename ObservedT, typename Observer>
bool WindowRuntimeContext::observe_entity(
    EntityHandle<ObserverT> observer_entity,
    EntityHandle<ObservedT> observed_entity,
    Observer&& observer) const {
  const std::uintptr_t context_token = detail::entity_context_token(*this);
  if (observer_entity.empty() || observed_entity.empty() ||
      !observer_entity.matches_context(context_token) ||
      !observed_entity.matches_context(context_token) ||
      read_entity(observer_entity) == nullptr) {
    return false;
  }

  return observe_model(
      observed_entity.id(),
      [observer_entity, observer = std::forward<Observer>(observer)](
          const WindowRuntimeContext& context,
          Model<ObservedT> observed_model) mutable {
        if (!observer_entity.matches_context(
                detail::entity_context_token(context))) {
          return;
        }

        ObserverT* observing_state =
            context.runtime.template mutate_entity<ObserverT>(
                observer_entity.id());
        if (observing_state == nullptr) {
          return;
        }

        detail::invoke_entity_to_entity_observer(
            observer,
            *observing_state,
            context.entity(observed_model),
            context);
      });
}

template <typename ObserverT, typename ObservedT, typename Observer>
Subscription WindowRuntimeContext::observe_entity_subscription(
    EntityHandle<ObserverT> observer_entity,
    EntityHandle<ObservedT> observed_entity,
    Observer&& observer) const {
  const std::uintptr_t context_token = detail::entity_context_token(*this);
  if (observer_entity.empty() || observed_entity.empty() ||
      !observer_entity.matches_context(context_token) ||
      !observed_entity.matches_context(context_token) ||
      read_entity(observer_entity) == nullptr) {
    return {};
  }

  return observe_model_subscription(
      observed_entity.id(),
      [observer_entity, observer = std::forward<Observer>(observer)](
          const WindowRuntimeContext& context,
          Model<ObservedT> observed_model) mutable {
        if (!observer_entity.matches_context(
                detail::entity_context_token(context))) {
          return;
        }

        ObserverT* observing_state =
            context.runtime.template mutate_entity<ObserverT>(
                observer_entity.id());
        if (observing_state == nullptr) {
          return;
        }

        detail::invoke_entity_to_entity_observer(
            observer,
            *observing_state,
            context.entity(observed_model),
            context);
      });
}

template <typename T>
bool WindowRuntimeContext::remove_model(Model<T> model) const {
  return runtime.remove_entity(model);
}

template <typename T>
bool WindowRuntimeContext::remove_entity(EntityHandle<T> entity) const {
  if (entity.empty() ||
      !entity.matches_context(detail::entity_context_token(*this))) {
    return false;
  }

  return runtime.remove_entity(entity.id());
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
  const auto context_token = reinterpret_cast<std::uintptr_t>(this);
  if (entity.empty() || !entity.matches_context(context_token) ||
      read_entity(entity.id()) == nullptr) {
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

template <typename Observer>
bool WindowRuntime::observe_window(Observer&& observer) {
  WindowObservationCallback observer_fn{std::forward<Observer>(observer)};
  if (!observer_fn) {
    return false;
  }

  window_observers_.push_back(WindowObserver{
      .subscription_id = {},
      .runtime_id = root_window_runtime_id_,
      .callback = std::move(observer_fn),
  });
  return true;
}

template <typename Observer>
Subscription WindowRuntime::observe_window_subscription(Observer&& observer) {
  WindowObservationCallback observer_fn{std::forward<Observer>(observer)};
  if (!observer_fn) {
    return {};
  }

  const SubscriptionId subscription_id{next_subscription_id_++};
  window_observers_.push_back(WindowObserver{
      .subscription_id = subscription_id,
      .runtime_id = root_window_runtime_id_,
      .callback = std::move(observer_fn),
  });
  return Subscription(*this, subscription_id);
}

template <typename Observer>
bool WindowRuntime::observe_view(ViewId view_id, Observer&& observer) {
  ViewObservationCallback observer_fn{std::forward<Observer>(observer)};
  if (!observer_fn || view_id.value == 0 || find_view(view_id) == nullptr) {
    return false;
  }

  view_observers_.push_back(ViewObserver{
      .subscription_id = {},
      .view_id = view_id,
      .callback = std::move(observer_fn),
  });
  return true;
}

template <typename Observer>
Subscription WindowRuntime::observe_view_subscription(
    ViewId view_id,
    Observer&& observer) {
  ViewObservationCallback observer_fn{std::forward<Observer>(observer)};
  if (!observer_fn || view_id.value == 0 || find_view(view_id) == nullptr) {
    return {};
  }

  const SubscriptionId subscription_id{next_subscription_id_++};
  view_observers_.push_back(ViewObserver{
      .subscription_id = subscription_id,
      .view_id = view_id,
      .callback = std::move(observer_fn),
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
bool WindowRuntime::invalidate_entity(EntityId<T> entity_id) {
  if (entity_id.value == 0 || read_entity(entity_id) == nullptr) {
    return false;
  }

  const bool notified = notify_entity_changed(entity_id);
  if (!notified) {
    request_render();
  }
  return true;
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
