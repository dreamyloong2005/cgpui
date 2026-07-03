#include "ui_internal.hpp"

#include <utility>

namespace cgpui {

void WindowRuntimeContext::request_render() const {
  runtime.request_render();
}

void WindowRuntimeContext::request_layout() const {
  runtime.request_layout();
}

void WindowRuntimeContext::request_paint() const {
  runtime.request_paint();
}

void WindowRuntimeContext::defer(DeferredCallback callback) const {
  runtime.defer(std::move(callback));
}

TimerId WindowRuntimeContext::schedule_timer(
    std::uint64_t delay_ms,
    TimerCallback callback) const {
  return runtime.schedule_timer(delay_ms, std::move(callback));
}

TimerId WindowRuntimeContext::schedule_repeating_timer(
    std::uint64_t interval_ms,
    TimerCallback callback) const {
  return runtime.schedule_repeating_timer(interval_ms, std::move(callback));
}

AnimationHandle WindowRuntimeContext::start_animation(
    AnimationOptions options,
    AnimationCallback callback) const {
  return runtime.start_animation(options, std::move(callback));
}

std::optional<AnimationSnapshot> WindowRuntimeContext::animation_snapshot(
    AnimationId id) const {
  return runtime.animation_snapshot(id);
}

bool WindowRuntimeContext::cancel_animation(AnimationId id) const {
  return runtime.cancel_animation(id);
}

TaskHandle WindowRuntimeContext::spawn_task(
    TaskCompletionCallback callback) const {
  return runtime.spawn_task(std::move(callback));
}

TaskHandle WindowRuntimeContext::spawn_background_task(
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  return runtime.spawn_background_task(
      std::move(work),
      std::move(completion));
}

void WindowRuntimeContext::batch_updates(
    UpdateBatchCallback callback) const {
  runtime.batch_updates(std::move(callback));
}

void WindowRuntimeContext::clear_invalidation() const {
  runtime.clear_invalidation();
}

InvalidationState WindowRuntimeContext::invalidation_state() const {
  return runtime.invalidation_state();
}

RuntimeDiagnosticsSnapshot WindowRuntimeContext::diagnostics_snapshot() const {
  return runtime.diagnostics_snapshot();
}

std::span<const PlatformDiagnosticEvent>
WindowRuntimeContext::platform_diagnostics() const {
  return runtime.platform_diagnostics();
}

} // namespace cgpui
