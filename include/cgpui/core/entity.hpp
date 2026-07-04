#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>

namespace cgpui {

template <typename T>
struct EntityId {
  std::uint64_t value = 0;

  friend bool operator==(EntityId, EntityId) = default;
};

template <typename T>
using Entity = EntityId<T>;

template <typename T>
using Model = EntityId<T>;

template <typename T>
class WeakEntity;

template <typename T>
class EntityHandle {
 public:
  constexpr EntityHandle() = default;
  constexpr explicit EntityHandle(EntityId<T> id) : id_(id) {}

  [[nodiscard]] constexpr EntityId<T> id() const {
    return id_;
  }

  [[nodiscard]] constexpr bool empty() const {
    return id_.value == 0;
  }

  [[nodiscard]] constexpr WeakEntity<T> downgrade() const;

  template <typename Context>
  [[nodiscard]] const T* read(const Context& context) const {
    return context.read_entity(id_);
  }

  template <typename Context, typename Update>
  bool update(const Context& context, Update&& update) const {
    return context.update_model(id_, std::forward<Update>(update));
  }

  template <typename Context, typename Observer>
  bool observe(const Context& context, Observer&& observer) const {
    return context.observe_entity(*this, std::forward<Observer>(observer));
  }

  template <typename Context, typename Observer>
  [[nodiscard]] auto observe_subscription(
      const Context& context,
      Observer&& observer) const {
    return context.observe_entity_subscription(
        *this,
        std::forward<Observer>(observer));
  }

  friend bool operator==(const EntityHandle&, const EntityHandle&) = default;

 private:
  EntityId<T> id_;
};

template <typename T>
class WeakEntity {
 public:
  constexpr WeakEntity() = default;
  constexpr explicit WeakEntity(EntityId<T> id) : id_(id) {}

  [[nodiscard]] constexpr EntityId<T> id() const {
    return id_;
  }

  [[nodiscard]] constexpr bool empty() const {
    return id_.value == 0;
  }

  template <typename Context>
  [[nodiscard]] std::optional<EntityHandle<T>> upgrade(
      const Context& context) const {
    const auto upgraded = context.upgrade_entity(*this);
    if (!upgraded.has_value()) {
      return std::nullopt;
    }
    return EntityHandle<T>(*upgraded);
  }

  template <typename Context>
  [[nodiscard]] const T* read(const Context& context) const {
    const std::optional<EntityHandle<T>> upgraded = upgrade(context);
    if (!upgraded.has_value()) {
      return nullptr;
    }
    return upgraded->read(context);
  }

  friend bool operator==(const WeakEntity&, const WeakEntity&) = default;

 private:
  EntityId<T> id_;
};

template <typename T>
constexpr WeakEntity<T> EntityHandle<T>::downgrade() const {
  return WeakEntity<T>(id_);
}

template <typename T>
class EntityStore {
 public:
  template <typename... Args>
  EntityId<T> emplace(Args&&... args) {
    const EntityId<T> id{next_id_++};
    entities_.emplace(id.value, T{std::forward<Args>(args)...});
    return id;
  }

  EntityId<T> insert(T entity) {
    const EntityId<T> id{next_id_++};
    entities_.emplace(id.value, std::move(entity));
    return id;
  }

  [[nodiscard]] T* get(EntityId<T> id) {
    const auto entry = entities_.find(id.value);
    if (entry == entities_.end()) {
      return nullptr;
    }
    return &entry->second;
  }

  [[nodiscard]] const T* get(EntityId<T> id) const {
    const auto entry = entities_.find(id.value);
    if (entry == entities_.end()) {
      return nullptr;
    }
    return &entry->second;
  }

  bool remove(EntityId<T> id) {
    return entities_.erase(id.value) > 0;
  }

 private:
  std::uint64_t next_id_ = 1;
  std::unordered_map<std::uint64_t, T> entities_;
};

} // namespace cgpui
