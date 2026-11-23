#include "runtime_frame_readiness_test_support.hpp"

using namespace readiness_test;
using cgpui::ErrorCode;

namespace {
int early_success(bool child) {
  Fixture f;
  const int result = f.run([&] {
    const auto target = child ? f.open_child() : f.app.root;
    if (!target) return;
    target->outcomes = {ErrorCode::frame_not_ready};
    f.paint(target);
    if (!f.require(f.errors == 0, "pending treated as fatal")) return;
    f.runtime.set_after_frame_callback([&](const auto&) {
      ++f.after_frames;
      // Reentrant clock advancement exposes cancellation AFTER user callbacks.
      f.app.advance(2);
      f.runtime.advance_time(0);
    });
    f.paint(target);
    f.require(f.app.pump() && f.after_frames == 1 && target->begins == 2 &&
        target->redraws == 0 && !f.app.wake_deadline,
        "successful frame must cancel its timer before after-frame callbacks");
  });
  return result || f.failure ? 1 : 0;
}

int reentrant_timer_registration(bool child) {
  Fixture f;
  const int result = f.run([&] {
    const auto target = child ? f.open_child() : f.app.root;
    if (!target) return;
    target->outcomes = {ErrorCode::frame_not_ready, ErrorCode::frame_not_ready};
    f.app.on_next_timer_request = [&] {
      // Deliver only the due transport wake inside timer registration. Leave
      // ordinary redraw queued until registration returns to expose stale ids.
      f.app.advance(2);
      f.app.wake_deadline.reset();
      f.app.emit(f.app.root, cgpui::WindowWakeupRequested{});
    };
    f.paint(target);
    if (!f.require(f.errors == 0 && target->begins == 1 && target->redraws == 1,
                    "reentrant timer delivery did not queue ordinary redraw")) return;
    f.require(f.app.pump() && target->begins == 2 && f.app.wake_deadline == 4,
              "consumed timer id was restored and prevented a later pending rearm");
    f.app.advance(2);
    f.require(f.app.pump() && target->begins == 3 && target->presents == 1,
              "reentrant transport stalled the pending window");
  });
  return result || f.failure ? 1 : 0;
}

int close_during_success_cancellation(bool child) {
  Fixture f;
  unsigned user_timer_fired = 0;
  const int result = f.run([&] {
    const auto target = child ? f.open_child() : f.app.root;
    if (!target) return;
    target->outcomes = {ErrorCode::frame_not_ready};
    f.paint(target);
    if (!f.require(f.errors == 0, "pending treated as fatal")) return;
    const auto user_timer = f.runtime.schedule_timer(100,
        [&](const auto&) { ++user_timer_fired; });
    if (!f.require(user_timer.value != 0, "late independent timer unavailable")) return;
    f.app.on_next_timer_request = [&] {
      // Successful frame cancellation reschedules the late user deadline. This
      // platform reentrancy can detach a child or accept root close immediately.
      f.app.emit(target, cgpui::WindowCloseRequested{});
    };
    f.paint(target);
    f.require(f.errors == 0 && target->begins == 2 && target->presents == 1 &&
        target->control_region_publications == 0 && f.after_frames == 0 &&
        f.app.quits == (child ? 0U : 1U),
        "frame continued publishing after cancellation reentered window close");
    if (child) {
      f.app.advance(100);
      f.require(f.app.pump() && target->destroyed && user_timer_fired == 1 &&
          target->redraws == 0 && f.app.root->begins == 0,
          "child cancellation touched detached resources or removed an unrelated timer");
    }
  });
  return result || f.failure ? 1 : 0;
}

int outcome_after_pending(bool child, cgpui::ErrorCode outcome) {
  Fixture f;
  const bool fatal = outcome != ErrorCode::frame_retry_required;
  const int result = f.run([&] {
    const auto target = child ? f.open_child() : f.app.root;
    if (!target) return;
    target->outcomes = {ErrorCode::frame_not_ready, outcome};
    f.paint(target);
    if (!f.require(f.errors == 0, "initial pending treated as fatal")) return;
    f.paint(target);
    f.require(!f.app.wake_deadline, "recovery/fatal outcome retained a pacing timer");
    if (fatal) {
      f.require(f.errors == 1 && f.app.quits == 1 && target->redraws == 0,
                "real fatal outcome must not be converted to a delayed retry");
    } else {
      f.require(f.app.pump() && target->begins == 3 && target->redraws == 1 &&
          f.after_frames == 1 && f.app.now == 0,
          "typed recovery retry must remain immediate, not use pending delay");
      f.app.advance(20);
      f.require(f.app.pump() && target->begins == 3, "recovery left a stale pending wake");
    }
  });
  return result != (fatal ? 1 : 0) || f.failure ? 1 : 0;
}
} // namespace

int main() {
  for (const bool child : {false, true}) {
    if (early_success(child)) return 1;
    if (reentrant_timer_registration(child)) return 4;
    if (close_during_success_cancellation(child)) return 5;
    if (outcome_after_pending(child, ErrorCode::frame_retry_required)) return 2;
    if (outcome_after_pending(child, ErrorCode::frame_acquisition_failed)) return 3;
  }
  return 0;
}
