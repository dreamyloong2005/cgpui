#include "runtime_frame_readiness_test_support.hpp"

using namespace readiness_test;
using cgpui::ErrorCode;

int main() {
  for (const bool child : {false, true}) {
    Fixture f;
    std::shared_ptr<State> retired;
    const int result = f.run([&] {
      retired = child ? f.open_child() : f.app.root;
      if (!retired) return;
      retired->resize_outcomes = {ErrorCode::renderer_recovery_pending};
      f.app.emit(retired, cgpui::WindowResized{.size = {250, 125}, .scale = cgpui::DpiScale{1}});
      const auto attempts = retired->resizes;
      f.app.emit(retired, cgpui::WindowCloseRequested{});
      f.app.advance(100);
      f.require(f.app.pump() && retired->resizes == attempts && retired->begins == 0 &&
          f.errors == 0, "closed window retained a recovery resize or redraw");
    });
    if (result || f.failure) return 1;
    const int second = f.run([&] {
      f.app.advance(100);
      f.require(f.app.pump() && f.app.root->begins == 0 && f.app.root->resizes == 0,
          "old desired extent escaped into the replacement runtime session");
      f.paint(f.app.root);
      f.require(f.after_frames == 1 && f.errors == 0, "new session cannot render after cancellation");
    });
    if (second || f.failure) return 2;
  }
  for (const bool child : {false, true}) {
    Fixture f;
    const int result = f.run([&] {
      const auto target = child ? f.open_child() : f.app.root;
      if (!target) return;
      target->resize_outcomes = {ErrorCode::invalid_argument};
      f.app.emit(target, cgpui::WindowResized{.size = {250, 125}, .scale = cgpui::DpiScale{1}});
      f.require(f.errors == 1 && f.app.quits == 1 && !f.app.wake_deadline,
          "root and child fatal resize errors must follow the same policy");
    });
    if (result != 1 || f.failure) return 3;
  }
  {
    Fixture f;
    const int result = f.run([&] {
      f.runtime.set_after_render_callback([&](const auto&, const auto&) {
        f.app.emit(f.app.root, cgpui::WindowCloseRequested{});
      });
      f.paint(f.app.root);
      f.require(f.errors == 0 && f.app.quits == 1 && f.app.root->begins == 0 &&
          f.app.root->presents == 0 && f.after_frames == 0 && !f.app.wake_deadline,
          "root close during view callback must prevent frame acquisition and success");
    });
    if (result || f.failure) return 4;
  }
  return 0;
}
