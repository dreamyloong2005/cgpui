#include "cgpui/ui/async_context.hpp"

#include "cgpui/ui/runtime_context.hpp"

#include <utility>

namespace cgpui {

void AsyncContextCapability::defer(DeferredCallback callback) const {
  context_->defer(std::move(callback));
}

TimerId AsyncContextCapability::schedule_timer(
    std::uint64_t delay_ms,
    TimerCallback callback) const {
  return context_->schedule_timer(delay_ms, std::move(callback));
}

TimerId AsyncContextCapability::schedule_repeating_timer(
    std::uint64_t interval_ms,
    TimerCallback callback) const {
  return context_->schedule_repeating_timer(interval_ms, std::move(callback));
}

AnimationHandle AsyncContextCapability::start_animation(
    AnimationOptions options,
    AnimationCallback callback) const {
  return context_->start_animation(options, std::move(callback));
}

std::optional<AnimationSnapshot> AsyncContextCapability::animation_snapshot(
    AnimationId id) const {
  return context_->animation_snapshot(id);
}

bool AsyncContextCapability::cancel_animation(AnimationId id) const {
  return context_->cancel_animation(id);
}

TaskHandle AsyncContextCapability::spawn_task(
    TaskCompletionCallback callback) const {
  return context_->spawn_task(std::move(callback));
}

TaskHandle AsyncContextCapability::spawn_background_task(
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  return context_->spawn_background_task(
      std::move(work),
      std::move(completion));
}

void AsyncContextCapability::batch_updates(
    UpdateBatchCallback callback) const {
  context_->batch_updates(std::move(callback));
}

AsyncContextCapability WindowRuntimeContext::async_context() const {
  return AsyncContextCapability(*this);
}

} // namespace cgpui
