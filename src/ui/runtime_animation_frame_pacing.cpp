#include "ui_internal.hpp"

#include <algorithm>
#include <limits>

namespace cgpui {
namespace {

std::uint64_t add_interval(
    std::uint64_t deadline,
    std::uint64_t interval,
    std::uint64_t now) {
  interval = interval == 0 ? 16 : interval;
  if (deadline == 0) {
    return interval > std::numeric_limits<std::uint64_t>::max() - now
        ? std::numeric_limits<std::uint64_t>::max()
        : now + interval;
  }
  if (deadline > now) return deadline;
  const std::uint64_t steps = (now - deadline) / interval + 1;
  return steps > (std::numeric_limits<std::uint64_t>::max() - deadline) /
                     interval
      ? std::numeric_limits<std::uint64_t>::max()
      : deadline + steps * interval;
}

} // namespace

std::uint64_t WindowRuntime::next_animation_frame_deadline(
    std::uint64_t deadline,
    std::uint64_t interval_ms) const {
  return add_interval(deadline, interval_ms, current_time_ms_);
}

AnimationFramePacingSnapshot
WindowRuntime::animation_frame_pacing_snapshot() const {
  AnimationFramePacingSnapshot snapshot = animation_frame_pacing_diagnostics_;
  std::uint64_t next_deadline = element_animation_frame_pending_
      ? element_animation_frame_deadline_ms_
      : std::numeric_limits<std::uint64_t>::max();
  for (const RuntimeAnimation& animation : animations_) {
    if (!animation.frame_pending || animation.complete || animation.cancelled) {
      continue;
    }
    snapshot.pending_animation_count += 1;
    next_deadline = std::min(next_deadline, animation.next_tick_ms);
  }
  snapshot.element_frame_pending = element_animation_frame_pending_;
  snapshot.wakeup_scheduled = animation_frame_timer_id_.value != 0;
  snapshot.next_frame_deadline_ms =
      next_deadline == std::numeric_limits<std::uint64_t>::max()
      ? 0
      : next_deadline;
  return snapshot;
}

void WindowRuntime::schedule_animation_frame_wakeup(bool count_coalescing) {
  const AnimationFramePacingSnapshot pacing = animation_frame_pacing_snapshot();
  if (delivering_animation_frame_ || window_ == nullptr || should_quit_) return;
  if (pacing.next_frame_deadline_ms == 0) {
    if (animation_frame_timer_id_.value != 0) {
      (void)cancel_timer(animation_frame_timer_id_);
      animation_frame_timer_id_ = {};
    }
    animation_frame_timer_deadline_ms_ = 0;
    return;
  }
  if (animation_frame_timer_id_.value != 0 &&
      animation_frame_timer_deadline_ms_ == pacing.next_frame_deadline_ms) {
    animation_frame_pacing_diagnostics_.coalesced_request_count +=
        count_coalescing ? 1U : 0U;
    return;
  }
  if (animation_frame_timer_id_.value != 0) {
    (void)cancel_timer(animation_frame_timer_id_);
    animation_frame_timer_id_ = {};
  }
  const std::uint64_t delay = pacing.next_frame_deadline_ms > current_time_ms_
      ? pacing.next_frame_deadline_ms - current_time_ms_
      : 0;
  animation_frame_timer_deadline_ms_ = pacing.next_frame_deadline_ms;
  animation_frame_pacing_diagnostics_.last_scheduled_delay_ms = delay;
  animation_frame_pacing_diagnostics_.scheduled_wakeup_count += 1;
  animation_frame_timer_id_ = schedule_timer(
      delay,
      [this](const WindowRuntimeContext&) {
        animation_frame_timer_id_ = {};
        deliver_animation_frame();
      });
}

void WindowRuntime::request_element_animation_frame(
    std::uint64_t interval_ms) {
  element_animation_frame_deadline_ms_ = add_interval(
      element_animation_frame_deadline_ms_, interval_ms, current_time_ms_);
  element_animation_frame_pending_ = true;
  schedule_animation_frame_wakeup(true);
}

void WindowRuntime::clear_element_animation_frame() {
  element_animation_frame_pending_ = false;
  element_animation_frame_deadline_ms_ = 0;
  schedule_animation_frame_wakeup(false);
}

void WindowRuntime::deliver_animation_frame() {
  const std::uint64_t delivered_deadline = animation_frame_timer_deadline_ms_;
  animation_frame_timer_deadline_ms_ = 0;
  animation_frame_pacing_diagnostics_.delivered_frame_count += 1;
  animation_frame_pacing_diagnostics_.late_frame_count +=
      current_time_ms_ > delivered_deadline ? 1U : 0U;
  delivering_animation_frame_ = true;
  for (RuntimeAnimation& animation : animations_) {
    if (!animation.frame_pending || animation.complete || animation.cancelled ||
        animation.next_tick_ms > current_time_ms_) continue;
    animation.due_for_frame = true;
    animation.next_tick_ms = next_animation_frame_deadline(
        animation.next_tick_ms,
        animation.options.tick_interval_ms);
  }
  const bool deliver_element_frame = element_animation_frame_pending_ &&
      element_animation_frame_deadline_ms_ <= current_time_ms_;
  if (deliver_element_frame) element_animation_frame_pending_ = false;

  while (true) {
    if (should_quit_) {
      for (RuntimeAnimation& animation : animations_) {
        animation.due_for_frame = false;
      }
      break;
    }
    const auto due = std::find_if(
        animations_.begin(), animations_.end(),
        [](const RuntimeAnimation& animation) {
          return animation.due_for_frame;
        });
    if (due == animations_.end()) break;
    const AnimationId id = due->id;
    due->due_for_frame = false;
    tick_animation(id);
  }
  if (deliver_element_frame && !should_quit_) request_render();
  delivering_animation_frame_ = false;
  schedule_animation_frame_wakeup(false);
}

} // namespace cgpui
