#include "runtime_frame_readiness_test_support.hpp"

using namespace readiness_test;
using cgpui::ErrorCode;

int main() {
  for (const bool child : {false, true}) {
    for (const bool present : {false, true}) {
      Fixture f;
      const int result = f.run([&] {
        const auto target = child ? f.open_child() : f.app.root;
        if (!target) return;
        if (present) target->present_error = ErrorCode::renderer_recovery_pending;
        else target->outcomes = {ErrorCode::renderer_recovery_pending};
        f.paint(target);
        f.require(f.errors == 0 && f.after_frames == 0 && target->begins == 1 &&
            target->presents == unsigned(present) && target->redraws == 0 &&
            f.app.wake_deadline.has_value(), "unfinished recovery must defer without success or immediate redraw");
        target->present_error.reset();
        f.app.advance(1);
        f.require(f.app.pump() && target->begins == 1, "recovery retry fired before its delay");
        f.app.advance(1);
        f.require(f.app.pump() && target->begins == 2 && f.after_frames == 1 &&
            f.errors == 0 && !f.app.wake_deadline, "recovery must retry a fresh frame and retire the wake");
      });
      if (result || f.failure) return 1;
    }
  }
  return 0;
}
