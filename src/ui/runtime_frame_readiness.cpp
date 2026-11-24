#include "ui_internal.hpp"
#include "runtime_frame_readiness_internal.hpp"
#include "runtime_frame_retry_internal.hpp"

#include <algorithm>
#include <iterator>
#include <limits>
#include <utility>

namespace cgpui {

bool WindowRuntime::RuntimeFrameReadiness::handle(
    WindowRuntime& runtime, WindowRuntimeId window, const Error& error) {
  if (error.code == ErrorCode::frame_not_ready ||
      error.code == ErrorCode::renderer_recovery_pending) {
    if (auto scheduled = defer(runtime, window); !scheduled)
      runtime.fail_and_quit(scheduled.error());
    return true;
  }
  clear(runtime, window);
  return runtime_retry_frame(runtime, window, error);
}

Result<void> WindowRuntime::RuntimeFrameReadiness::defer(
    WindowRuntime& runtime, WindowRuntimeId window) {
  const auto* record = runtime.find_window_runtime_record(window);
  if (runtime.state_->core.should_quit_ || runtime.state_->core.shutting_down_ ||
      !record || !record->active || !record->window || !record->renderer) {
    clear(runtime, window);
    return {}; // A callback may already have closed the target; do not resurrect it.
  }
  runtime.sync_platform_time();
  const auto now = runtime.state_->scheduling.current_time_ms_;
  auto found = std::find_if(pending_.begin(), pending_.end(),
      [window](const Pending& pending) { return pending.window == window; });
  if (found != pending_.end() && found->timer.value) return {};
  if (next_token_ == std::numeric_limits<std::uint64_t>::max()) {
    return std::unexpected(Error{ErrorCode::frame_acquisition_failed,
                                "frame readiness token exhausted"});
  }
  const auto token = ++next_token_;
  if (found == pending_.end()) {
    pending_.push_back({.window = window, .token = token, .since_ms = now});
    found = std::prev(pending_.end());
  } else {
    found->token = token;
    found->delivered = false;
  }
  const auto elapsed = now >= found->since_ms ? now - found->since_ms : 0;
  const std::uint64_t delay = elapsed < 250 ? 2 : 64;
  const TimerId timer = runtime.schedule_window_timer(window, delay, false,
      [token](const WindowRuntimeContext& context) {
        context.runtime.state_->scheduling.frame_readiness_.deliver(context, token);
      });
  // Scheduling can call the platform transport. Do not retain a vector iterator
  // across it; cancellation/reentry may have invalidated the original entry.
  found = std::find_if(pending_.begin(), pending_.end(),
      [window, token](const Pending& pending) {
        return pending.window == window && pending.token == token;
      });
  if (found == pending_.end()) {
    if (timer.value) (void)runtime.cancel_timer(timer);
    return {};
  }
  if (!timer.value) {
    pending_.erase(found);
    return std::unexpected(Error{ErrorCode::frame_acquisition_failed,
                                "frame readiness timer could not be scheduled"});
  }
  // A reentrant platform wake may already have consumed this one-shot while
  // its redraw is still queued. Never write that retired TimerId back.
  if (!found->delivered) found->timer = timer;
  return {};
}

void WindowRuntime::RuntimeFrameReadiness::deliver(
    const WindowRuntimeContext& context, std::uint64_t token) {
  auto found = std::find_if(pending_.begin(), pending_.end(),
      [&](const Pending& pending) {
        return pending.window == context.window_runtime_id && pending.token == token;
      });
  if (found == pending_.end()) return;
  auto& runtime = context.runtime;
  const auto* record = runtime.find_window_runtime_record(context.window_runtime_id);
  if (runtime.state_->core.should_quit_ || runtime.state_->core.shutting_down_ ||
      !record || !record->active || !record->window || !record->renderer) {
    clear(runtime, context.window_runtime_id);
    return;
  }
  found->delivered = true;
  found->timer = {}; // The one-shot has fired; a reentrant next pending gets a new token.
  context.request_render(); // Ordinary deferred redraw, never a direct draw/event injection.
}

void WindowRuntime::RuntimeFrameReadiness::clear(
    WindowRuntime& runtime, WindowRuntimeId window) {
  const auto found = std::find_if(pending_.begin(), pending_.end(),
      [window](const Pending& pending) { return pending.window == window; });
  if (found == pending_.end()) return;
  const TimerId timer = found->timer;
  pending_.erase(found); // Retire bookkeeping before cancellation calls platform transport.
  if (timer.value) (void)runtime.cancel_timer(timer);
}

void WindowRuntime::RuntimeFrameReadiness::clear_all(WindowRuntime& runtime) {
  runtime.state_->windows.surface_resizes_.clear_all();
  const auto retired = std::exchange(pending_, {});
  for (const auto& pending : retired)
    if (pending.timer.value) (void)runtime.cancel_timer(pending.timer);
}

} // namespace cgpui
