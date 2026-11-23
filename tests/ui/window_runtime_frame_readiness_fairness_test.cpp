#include "runtime_frame_readiness_test_support.hpp"

using namespace readiness_test;
using cgpui::ErrorCode;

int main() {
  Fixture f;
  unsigned user_timer = 0;
  const int result = f.run([&] {
    const auto root = f.app.root, child = f.open_child();
    if (!child) return;
    root->outcomes = {ErrorCode::frame_not_ready};
    child->outcomes = {ErrorCode::frame_not_ready};
    const auto user = f.runtime.schedule_timer(1, [&](const auto&) { ++user_timer; });
    f.require(user.value != 0, "user timer unavailable");
    f.paint(root);
    f.paint(child);
    if (!f.require(f.errors == 0 && f.app.wake_deadline == 1,
                    "pending timers replaced earlier user deadline")) return;
    f.app.emit(root, cgpui::WindowResized{.size = {200, 100}, .scale = {1}});
    f.app.emit(child, cgpui::WindowResized{.size = {240, 120}, .scale = {1}});
    f.app.advance(1);
    f.require(f.app.pump() && user_timer == 1 && root->begins == 1 && child->begins == 1,
              "readiness interfered with independent user timer");
    f.app.advance(1);
    f.require(f.app.pump() && root->presents == 1 && child->presents == 1 &&
        root->redraws == 1 && child->redraws == 1 && root->resizes == 1 && child->resizes == 1,
        "resize swallowed the only future wake or a window starved its peer");
  });
  return result || f.failure ? 1 : 0;
}
