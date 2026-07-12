#pragma once

#include "cgpui/ui/animation_curve.hpp"
#include "cgpui/ui/runtime_callbacks.hpp"
#include "cgpui/ui/runtime_ids.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

namespace cgpui {

class TaskCancellationToken {
 public:
  TaskCancellationToken() = default;

  [[nodiscard]] bool cancellation_requested() const;

 private:
  friend class WindowRuntime;

  explicit TaskCancellationToken(std::shared_ptr<std::atomic_bool> state)
      : state_(std::move(state)) {}

  std::shared_ptr<std::atomic_bool> state_;
};

using BackgroundTaskCallback = std::function<void(TaskCancellationToken)>;

struct AnimationOptions {
  std::uint64_t duration_ms = 0;
  AnimationEasing easing = AnimationEasing::linear;
  AnimationCurve curve;
  std::uint64_t tick_interval_ms = 16;
};

struct AnimationSnapshot {
  AnimationId id;
  std::uint64_t elapsed_ms = 0;
  std::uint64_t duration_ms = 0;
  float linear_progress = 0.0F;
  float eased_progress = 0.0F;
  AnimationEasing easing = AnimationEasing::linear;
  AnimationCurve curve;
  bool complete = false;
};

using AnimationCallback = std::function<void(
    const WindowRuntimeContext&,
    const AnimationSnapshot&)>;

class AnimationHandle {
 public:
  AnimationHandle() = default;

  [[nodiscard]] AnimationId id() const { return id_; }
  [[nodiscard]] bool active() const;
  [[nodiscard]] bool complete() const;
  [[nodiscard]] std::optional<AnimationSnapshot> progress() const;
  [[nodiscard]] bool cancel();

 private:
  friend class WindowRuntime;

  AnimationHandle(WindowRuntime& runtime, AnimationId id)
      : runtime_(&runtime), id_(id) {}

  WindowRuntime* runtime_ = nullptr;
  AnimationId id_{};
};

class TaskHandle {
 public:
  TaskHandle() = default;

  [[nodiscard]] TaskId id() const { return id_; }
  [[nodiscard]] bool active() const;
  [[nodiscard]] bool complete() const;
  [[nodiscard]] bool cancelled() const;
  [[nodiscard]] bool cancel();

 private:
  friend class WindowRuntime;

  TaskHandle(WindowRuntime& runtime, TaskId id) : runtime_(&runtime), id_(id) {}

  WindowRuntime* runtime_ = nullptr;
  TaskId id_{};
};

class Subscription {
 public:
  Subscription() = default;
  ~Subscription();

  Subscription(const Subscription&) = delete;
  Subscription& operator=(const Subscription&) = delete;

  Subscription(Subscription&& other) noexcept;
  Subscription& operator=(Subscription&& other) noexcept;

  [[nodiscard]] SubscriptionId id() const { return id_; }
  [[nodiscard]] bool connected() const;
  [[nodiscard]] bool release();

 private:
  friend class WindowRuntime;

  Subscription(WindowRuntime& runtime, SubscriptionId id)
      : runtime_(&runtime), id_(id) {}

  WindowRuntime* runtime_ = nullptr;
  SubscriptionId id_{};
};

} // namespace cgpui
