#include "ui_internal.hpp"
#include "runtime_task_pool_internal.hpp"
#include "runtime_window_store_internal.hpp"

namespace cgpui {

std::optional<RenderRecord> WindowRuntime::last_render_record() const {
  return state_->diagnostics.last_render_record_;
}

RuntimeDiagnosticsSnapshot WindowRuntime::diagnostics_snapshot() const {
  std::size_t connected_subscription_count = 0;
  for (const EntityObserver& observer : state_->entities.entity_observers_) {
    if (observer.subscription_id.value != 0 && observer.callback) {
      connected_subscription_count += 1;
    }
  }
  for (const WindowObserver& observer : state_->entities.window_observers_) {
    if (observer.subscription_id.value != 0 && observer.callback) {
      connected_subscription_count += 1;
    }
  }
  for (const ViewObserver& observer : state_->entities.view_observers_) {
    if (observer.subscription_id.value != 0 && observer.callback) {
      connected_subscription_count += 1;
    }
  }
  std::size_t active_runtime_window_count = 0;
  for (const WindowRuntimeRecord& record : state_->windows.window_runtime_records_->records()) {
    active_runtime_window_count += record.active ? 1U : 0U;
  }
  const RuntimeTaskDiagnostics task_counts = task_diagnostics();
  const RuntimeTaskPool::Snapshot task_pool = state_->tasks.task_pool_->snapshot();
  std::size_t active_animation_count = 0;
  std::size_t completed_animation_count = 0;
  std::size_t cancelled_animation_count = 0;
  for (const RuntimeAnimation& animation : state_->animations.animations_) {
    active_animation_count +=
        !animation.complete && !animation.cancelled ? 1U : 0U;
    completed_animation_count += animation.complete ? 1U : 0U;
    cancelled_animation_count += animation.cancelled ? 1U : 0U;
  }

  return RuntimeDiagnosticsSnapshot{
      .entity_store_count = state_->entities.entity_stores_.size(),
      .entity_count = state_->entities.entity_count_,
      .view_entity_subscription_count = state_->entities.entity_subscriptions_.size(),
      .entity_observer_count = state_->entities.entity_observers_.size(),
      .window_observer_count = state_->entities.window_observers_.size(),
      .view_observer_count = state_->entities.view_observers_.size(),
      .connected_subscription_count = connected_subscription_count,
      .runtime_window_record_count = state_->windows.window_runtime_records_->size(),
      .active_runtime_window_count = active_runtime_window_count,
      .opened_window_count = state_->windows.app_opened_windows_.size(),
      .active_native_child_window_count = state_->windows.native_additional_windows_.size(),
      .retired_native_child_window_count = state_->windows.retired_native_windows_.size(),
      .invalidation = state_->scheduling.invalidation_state_,
      .frame_index = state_->diagnostics.frame_index_,
      .last_render_record = state_->diagnostics.last_render_record_,
      .last_frame_statistics = state_->diagnostics.last_frame_statistics_,
      .last_renderer_frame_diagnostics = state_->diagnostics.last_renderer_frame_diagnostics_,
      .platform_diagnostics = state_->diagnostics.platform_diagnostics_,
      .animation_count = state_->animations.animations_.size(),
      .active_animation_count = active_animation_count,
      .completed_animation_count = completed_animation_count,
      .cancelled_animation_count = cancelled_animation_count,
      .last_animation_cancellation = state_->animations.last_animation_cancellation_,
      .animation_frame_pacing = animation_frame_pacing_snapshot(),
      .task_count = task_counts.task_count,
      .active_task_count = task_counts.active_task_count,
      .queued_task_count = task_counts.queued_task_count,
      .completed_task_count = task_counts.completed_task_count,
      .cancelled_task_count = task_counts.cancelled_task_count,
      .background_task_count = task_counts.background_task_count,
      .task_pool_worker_count = task_pool.worker_count,
      .task_pool_queued_work_count = task_pool.queued_work_count,
      .task_pool_active_work_count = task_pool.active_work_count,
      .task_pool_peak_active_work_count = task_pool.peak_active_work_count,
      .task_pool_completed_work_count = task_pool.completed_work_count,
  };
}

std::span<const PlatformDiagnosticEvent> WindowRuntime::platform_diagnostics()
    const {
  return state_->diagnostics.platform_diagnostics_;
}

WindowRuntime::RuntimeTaskDiagnostics WindowRuntime::task_diagnostics() const {
  RuntimeTaskDiagnostics diagnostics;
  std::lock_guard lock(state_->tasks.tasks_mutex_);
  diagnostics.task_count = state_->tasks.tasks_.size();
  diagnostics.queued_task_count = state_->tasks.task_completion_queue_.size();
  for (const RuntimeTask& task : state_->tasks.tasks_) {
    if (task.background) {
      diagnostics.background_task_count += 1;
    }
    if (task.completed) {
      diagnostics.completed_task_count += 1;
    }
    if (task.cancelled) {
      diagnostics.cancelled_task_count += 1;
    }
    if (!task.queued && !task.completed && !task.cancelled) {
      diagnostics.active_task_count += 1;
    }
  }
  return diagnostics;
}

} // namespace cgpui
