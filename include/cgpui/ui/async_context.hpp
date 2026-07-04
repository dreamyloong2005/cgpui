#pragma once

#include "cgpui/ui/runtime_callbacks.hpp"
#include "cgpui/ui/runtime_handles.hpp"

#include <cstdint>
#include <optional>

namespace cgpui {

struct WindowRuntimeContext;

class AsyncContextCapability {
 public:
  constexpr AsyncContextCapability() = default;
  constexpr explicit AsyncContextCapability(
      const WindowRuntimeContext& context)
      : context_(&context) {}

  void defer(DeferredCallback callback) const;
  [[nodiscard]] TimerId schedule_timer(
      std::uint64_t delay_ms,
      TimerCallback callback) const;
  [[nodiscard]] TimerId schedule_repeating_timer(
      std::uint64_t interval_ms,
      TimerCallback callback) const;

  [[nodiscard]] AnimationHandle start_animation(
      AnimationOptions options,
      AnimationCallback callback) const;
  [[nodiscard]] std::optional<AnimationSnapshot> animation_snapshot(
      AnimationId id) const;
  [[nodiscard]] bool cancel_animation(AnimationId id) const;

  [[nodiscard]] TaskHandle spawn_task(TaskCompletionCallback callback) const;
  [[nodiscard]] TaskHandle spawn_background_task(
      BackgroundTaskCallback work,
      TaskCompletionCallback completion) const;
  void batch_updates(UpdateBatchCallback callback) const;

 private:
  const WindowRuntimeContext* context_ = nullptr;
};

} // namespace cgpui
