#include "runtime_frame_readiness_test_support.hpp"

using namespace readiness_test;
using cgpui::ErrorCode;

int main() {
  for (const bool child : {false, true}) {
    for (const auto outcome : {ErrorCode::frame_not_ready, ErrorCode::invalid_argument}) {
      Fixture f;
      const int result = f.run([&] {
        const auto target = child ? f.open_child() : f.app.root;
        if (!target) return;
        target->present_error = outcome;
        f.paint(target);
        f.require(f.errors == 1 && f.app.quits == 1 && f.after_frames == 0 &&
            target->begins == 1 && target->presents == 1 && target->redraws == 0 &&
            !f.app.wake_deadline, "Present failure must stay fatal, never begin-only admission");
        const bool invalid_pending = outcome == ErrorCode::frame_not_ready;
        f.require(f.last_error == (invalid_pending ? ErrorCode::frame_acquisition_failed : outcome) &&
            f.last_error_message == (invalid_pending
                ? "present violated begin-only frame_not_ready contract: scripted present outcome"
                : "scripted present outcome"),
            "Present pending must be normalized with its original message; other errors stay intact");
      });
      if (result != 1 || f.failure) return 1;
    }
  }
  return 0;
}
