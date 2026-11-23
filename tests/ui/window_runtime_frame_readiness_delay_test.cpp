#include "runtime_frame_readiness_test_support.hpp"

using namespace readiness_test;
using cgpui::ErrorCode;

namespace {
int delayed(bool child) {
  Fixture f;
  const int result = f.run([&] {
    const auto target = child ? f.open_child() : f.app.root;
    if (!target) return;
    target->outcomes = {ErrorCode::frame_not_ready, ErrorCode::frame_not_ready};
    f.paint(target);
    if (!f.require(f.errors == 0 && f.app.quits == 0 && f.after_frames == 0 &&
        target->begins == 1 && target->paints == 0 && target->presents == 0 && target->redraws == 0,
        "pending begin must park instead of failing, presenting or immediately redrawing")) return;
    f.require(f.app.wake_deadline == 2, "first pending must request a future wake");
    const auto timers = f.app.timer_requests;
    f.paint(target); // Extra externally requested paint must not duplicate/postpone the timer.
    f.require(f.app.timer_requests == timers && f.app.wake_deadline == 2,
              "repeated pending must coalesce onto the original deadline");
    f.app.advance(1);
    f.require(f.app.pump() && target->begins == 2 && target->redraws == 0,
              "pending frame retried before its deadline");
    f.app.advance(1);
    f.require(f.app.pump() && target->begins == 3 && target->presents == 1 &&
        target->redraws == 1 && target->paints == 1 && f.after_frames == 1 && !f.app.wake_deadline,
        "deadline must produce exactly one normal successful redraw");
    f.app.advance(100);
    f.require(f.app.pump() && target->begins == 3, "success left an obsolete pacing timer");
    if (child) f.require(f.app.root->begins == 0, "child pending incorrectly rendered root");
  });
  return result || f.failure ? 1 : 0;
}
} // namespace

int main() {
  for (const bool child : {false, true}) if (delayed(child)) return 1;
  return 0;
}
