#pragma once

#include "cgpui/core/entity.hpp"

#include <functional>
#include <memory>

namespace cgpui {

class AsyncContextCapability;
struct WindowRuntimeContext;

enum class CrossThreadEntityAccessStatus {
  read,
  updated,
  missing,
  context_mismatch,
};

using CrossThreadEntityAccessCompletion = std::function<void(
    CrossThreadEntityAccessStatus,
    const WindowRuntimeContext&)>;

namespace detail {

class CrossThreadEntityQueueState;
using CrossThreadEntityOperation =
    std::function<void(const WindowRuntimeContext&)>;

[[nodiscard]] bool enqueue_cross_thread_entity_operation(
    const std::weak_ptr<CrossThreadEntityQueueState>& queue,
    CrossThreadEntityOperation operation);

} // namespace detail

template <typename T>
class CrossThreadEntity {
 public:
  using ReadCallback =
      std::function<void(const T&, const WindowRuntimeContext&)>;
  using UpdateCallback =
      std::function<void(T&, const WindowRuntimeContext&)>;

  CrossThreadEntity() = default;

  [[nodiscard]] EntityId<T> id() const { return entity_.id(); }
  [[nodiscard]] bool empty() const { return entity_.empty(); }
  [[nodiscard]] bool attached() const { return !queue_.expired(); }

  [[nodiscard]] bool read(
      ReadCallback callback,
      CrossThreadEntityAccessCompletion completion = {}) const;
  [[nodiscard]] bool update(
      UpdateCallback callback,
      CrossThreadEntityAccessCompletion completion = {}) const;

  friend bool operator==(
      const CrossThreadEntity& lhs,
      const CrossThreadEntity& rhs) {
    return lhs.entity_ == rhs.entity_;
  }

 private:
  friend class AsyncContextCapability;

  CrossThreadEntity(
      EntityHandle<T> entity,
      std::weak_ptr<detail::CrossThreadEntityQueueState> queue)
      : entity_(entity), queue_(std::move(queue)) {}

  EntityHandle<T> entity_;
  std::weak_ptr<detail::CrossThreadEntityQueueState> queue_;
};

} // namespace cgpui
