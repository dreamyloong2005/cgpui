#include "cgpui/ui/test_context.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

#include <algorithm>
#include <mutex>

namespace cgpui {

void TestContextCapability::advance_time(std::uint64_t delta_ms) const {
  context_->runtime.advance_time(delta_ms);
}

void TestContextCapability::run_until_parked() const {
  WindowRuntime& runtime = context_->runtime;
  for (int iteration = 0; iteration < 1024 && !runtime.should_quit_;
       ++iteration) {
    bool has_queued_tasks = false;
    {
      std::lock_guard lock(runtime.tasks_mutex_);
      has_queued_tasks = !runtime.task_completion_queue_.empty();
    }
    const bool has_due_timer = std::ranges::any_of(
        runtime.timers_,
        [&](const WindowRuntime::RuntimeTimer& timer) {
          return timer.callback && timer.due_ms <= runtime.current_time_ms_;
        });
    if (!has_queued_tasks && !has_due_timer &&
        runtime.deferred_callbacks_.empty() &&
        !runtime.deferred_redraw_request_) {
      return;
    }
    context_->runtime.handle_wakeup();
  }
}

void TestContextCapability::advance_time_until_parked(
    std::uint64_t delta_ms) const {
  context_->runtime.advance_time(delta_ms);
  run_until_parked();
}

bool TestContextCapability::cancel_timer(TimerId id) const {
  return context_->runtime.cancel_timer(id);
}

bool TestContextCapability::complete_task(TaskId id) const {
  return context_->runtime.complete_task(id);
}

void TestContextCapability::drain_task_completions() const {
  context_->runtime.drain_task_completions();
}

} // namespace cgpui
