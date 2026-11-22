#include "ui_internal.hpp"

#include <utility>

namespace cgpui {

void WindowRuntimeContext::request_render() const {
  runtime.request_render_for_window(runtime.normalize_window_context_id(window_runtime_id));
}

void WindowRuntimeContext::request_layout() const {
  runtime.request_layout_for_window(runtime.normalize_window_context_id(window_runtime_id));
}

void WindowRuntimeContext::request_paint() const {
  runtime.request_paint_for_window(runtime.normalize_window_context_id(window_runtime_id));
}

void WindowRuntimeContext::defer(DeferredCallback callback) const {
  runtime.defer(runtime.bind_window_callback(window_runtime_id, std::move(callback)));
}

TimerId WindowRuntimeContext::schedule_timer(
    std::uint64_t delay_ms,
    TimerCallback callback) const {
  return runtime.schedule_window_timer(window_runtime_id, delay_ms, false, std::move(callback));
}

TimerId WindowRuntimeContext::schedule_repeating_timer(
    std::uint64_t interval_ms,
    TimerCallback callback) const {
  return runtime.schedule_window_timer(window_runtime_id, interval_ms, true, std::move(callback));
}

AnimationHandle WindowRuntimeContext::start_animation(
    AnimationOptions options,
    AnimationCallback callback) const {
  return runtime.start_animation(options, runtime.bind_window_animation_callback(window_runtime_id, std::move(callback)));
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
  return spawn_task(TaskPriority::normal, std::move(callback));
}

Result<TaskHandle> WindowRuntimeContext::try_spawn_task(
    TaskCompletionCallback callback) const {
  return try_spawn_task(TaskPriority::normal, std::move(callback));
}

TaskHandle WindowRuntimeContext::spawn_background_task(
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  return spawn_background_task(TaskPriority::normal, std::move(work), std::move(completion));
}

Result<TaskHandle> WindowRuntimeContext::try_spawn_background_task(
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  return try_spawn_background_task(TaskPriority::normal, std::move(work), std::move(completion));
}

void WindowRuntimeContext::batch_updates(
    UpdateBatchCallback callback) const {
  runtime.batch_updates(runtime.bind_window_callback(window_runtime_id, std::move(callback)));
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
