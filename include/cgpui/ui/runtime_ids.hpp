#pragma once

#include <cstdint>

namespace cgpui {

struct SubscriptionId {
  std::uint64_t value = 0;

  friend bool operator==(
      const SubscriptionId&,
      const SubscriptionId&) = default;
};

struct TimerId {
  std::uint64_t value = 0;

  friend bool operator==(const TimerId&, const TimerId&) = default;
};

struct AnimationId {
  std::uint64_t value = 0;

  friend bool operator==(const AnimationId&, const AnimationId&) = default;
};

struct TaskId {
  std::uint64_t value = 0;

  friend bool operator==(const TaskId&, const TaskId&) = default;
};

struct TaskGroupId {
  std::uint64_t value = 0;

  friend bool operator==(const TaskGroupId&, const TaskGroupId&) = default;
};

struct AsyncIoId {
  std::uint64_t value = 0;

  friend bool operator==(const AsyncIoId&, const AsyncIoId&) = default;
};

struct WindowRuntimeId {
  std::uint64_t value = 0;

  friend bool operator==(
      const WindowRuntimeId&,
      const WindowRuntimeId&) = default;
};

} // namespace cgpui
