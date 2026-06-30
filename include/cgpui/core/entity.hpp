#pragma once

#include <cstdint>
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

  friend bool operator==(const WeakEntity&, const WeakEntity&) = default;

 private:
  EntityId<T> id_;
};

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
