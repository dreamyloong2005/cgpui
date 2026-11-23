#include "runtime_frame_readiness_test_support.hpp"

using namespace readiness_test;
using cgpui::ErrorCode;

int main() {
  Fixture f;
  const int result = f.run([&] {
    const auto target = f.app.root;
    for (unsigned i = 0; i != 3; ++i) target->outcomes.push_back(ErrorCode::frame_not_ready);
    f.paint(target);
    if (!f.require(f.errors == 0 && f.app.pump(), "pending did not park")) return;
    f.app.advance(250); // One wake, not 125 synthesized old frames.
    f.require(f.app.pump() && target->begins == 2 && f.app.wake_deadline == 314,
              "sustained backpressure did not back off after one due attempt");
    f.app.advance(63);
    f.require(f.app.pump() && target->begins == 2, "backoff retried before the deadline");
    f.app.advance(1);
    f.require(f.app.pump() && target->begins == 3 && f.app.wake_deadline == 378,
              "continued pending must retain its backoff age");
    f.paint(target); // External paint can succeed before the backoff timer.
    f.require(target->presents == 1 && !f.app.wake_deadline,
              "early success did not cancel backoff");
    target->outcomes = {ErrorCode::frame_not_ready};
    f.paint(target);
    f.require(f.app.wake_deadline == 316, "success did not reset a new pending interval");
  });
  return result || f.failure ? 1 : 0;
}
