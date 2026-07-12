#include "ui_internal.hpp"
#include "runtime_task_pool_internal.hpp"

namespace cgpui {

std::optional<RenderRecord> WindowRuntime::last_render_record() const {
  return last_render_record_;
}

RuntimeDiagnosticsSnapshot WindowRuntime::diagnostics_snapshot() const {
  std::size_t connected_subscription_count = 0;
  for (const EntityObserver& observer : entity_observers_) {
    if (observer.subscription_id.value != 0 && observer.callback) {
      connected_subscription_count += 1;
    }
  }
  for (const WindowObserver& observer : window_observers_) {
    if (observer.subscription_id.value != 0 && observer.callback) {
      connected_subscription_count += 1;
    }
  }
  for (const ViewObserver& observer : view_observers_) {
    if (observer.subscription_id.value != 0 && observer.callback) {
      connected_subscription_count += 1;
    }
  }
  std::size_t active_runtime_window_count = 0;
  for (const WindowRuntimeRecord& record : window_runtime_records_) {
    active_runtime_window_count += record.active ? 1U : 0U;
  }
  const RuntimeTaskDiagnostics task_counts = task_diagnostics();
  const RuntimeTaskPool::Snapshot task_pool = task_pool_->snapshot();

  return RuntimeDiagnosticsSnapshot{
      .entity_store_count = entity_stores_.size(),
      .entity_count = entity_count_,
      .view_entity_subscription_count = entity_subscriptions_.size(),
      .entity_observer_count = entity_observers_.size(),
      .window_observer_count = window_observers_.size(),
      .view_observer_count = view_observers_.size(),
      .connected_subscription_count = connected_subscription_count,
      .runtime_window_record_count = window_runtime_records_.size(),
      .active_runtime_window_count = active_runtime_window_count,
      .opened_window_count = app_opened_windows_.size(),
      .active_native_child_window_count = native_additional_windows_.size(),
      .retired_native_child_window_count = retired_native_windows_.size(),
      .invalidation = invalidation_state_,
      .frame_index = frame_index_,
      .last_render_record = last_render_record_,
      .last_frame_statistics = last_frame_statistics_,
      .last_renderer_frame_diagnostics = last_renderer_frame_diagnostics_,
      .platform_diagnostics = platform_diagnostics_,
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
  return platform_diagnostics_;
}

WindowRuntime::RuntimeTaskDiagnostics WindowRuntime::task_diagnostics() const {
  RuntimeTaskDiagnostics diagnostics;
  std::lock_guard lock(tasks_mutex_);
  diagnostics.task_count = tasks_.size();
  diagnostics.queued_task_count = task_completion_queue_.size();
  for (const RuntimeTask& task : tasks_) {
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
