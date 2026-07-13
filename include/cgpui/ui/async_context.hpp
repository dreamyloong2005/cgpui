#pragma once

#include "cgpui/ui/async_io_hook.hpp"
#include "cgpui/ui/cross_thread_entity.hpp"
#include "cgpui/ui/runtime_callbacks.hpp"
#include "cgpui/ui/runtime_handles.hpp"
#include "cgpui/ui/task_priority.hpp"
#include "cgpui/ui/task_group.hpp"

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
  [[nodiscard]] constexpr bool valid() const { return context_ != nullptr; }

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
  [[nodiscard]] TaskGroup create_task_group() const;
  [[nodiscard]] AsyncIoHook create_async_io_hook(
      AsyncIoCompletionCallback callback) const;
  [[nodiscard]] AsyncIoHook create_async_io_hook(
      TaskPriority priority,
      AsyncIoCompletionCallback callback) const;
  template <typename T>
  [[nodiscard]] CrossThreadEntity<T> entity(EntityHandle<T> entity) const;

  [[nodiscard]] TaskHandle spawn_task(TaskCompletionCallback callback) const;
  [[nodiscard]] TaskHandle spawn_task(
      TaskPriority priority,
      TaskCompletionCallback callback) const;
  [[nodiscard]] Result<TaskHandle> try_spawn_task(
      TaskCompletionCallback callback) const;
  [[nodiscard]] Result<TaskHandle> try_spawn_task(
      TaskPriority priority,
      TaskCompletionCallback callback) const;
  [[nodiscard]] TaskHandle spawn_background_task(
      BackgroundTaskCallback work,
      TaskCompletionCallback completion) const;
  [[nodiscard]] TaskHandle spawn_background_task(
      TaskPriority priority,
      BackgroundTaskCallback work,
      TaskCompletionCallback completion) const;
  [[nodiscard]] Result<TaskHandle> try_spawn_background_task(
      BackgroundTaskCallback work,
      TaskCompletionCallback completion) const;
  [[nodiscard]] Result<TaskHandle> try_spawn_background_task(
      TaskPriority priority,
      BackgroundTaskCallback work,
      TaskCompletionCallback completion) const;
  void batch_updates(UpdateBatchCallback callback) const;

 private:
  const WindowRuntimeContext* context_ = nullptr;
};

} // namespace cgpui
