#include "runtime_frame_readiness_test_support.hpp"

using namespace readiness_test;
using cgpui::ErrorCode;

int main() {
  for (const bool child : {false, true}) {
    Fixture f;
    const int result = f.run([&] {
      const auto target = child ? f.open_child() : f.app.root;
      if (!target) return;
      const auto initial_resizes = target->resizes;
      target->resize_outcomes = {ErrorCode::renderer_recovery_pending,
          ErrorCode::renderer_recovery_pending, ErrorCode::renderer_recovery_pending, std::nullopt};
      f.app.emit(target, cgpui::WindowResized{.size = {200, 100}, .scale = cgpui::DpiScale{1}});
      f.app.emit(target, cgpui::WindowResized{.size = {600, 300}, .scale = cgpui::DpiScale{2}});
      f.paint(target);
      f.require(f.errors == 0 && target->begins == 0 && target->presents == 0 &&
          f.after_frames == 0 && target->resizes == initial_resizes + 3,
          "pending resize must block frame acquisition at the old extent");
      f.app.advance(2);
      f.require(f.app.pump() && f.errors == 0 && f.after_frames == 1 &&
          target->resizes == initial_resizes + 4 && target->begins == 1 &&
          target->last_resize_size.width == 600 && target->last_resize_size.height == 300 &&
          target->last_resize_scale.value == 2 && !f.app.wake_deadline,
          "latest desired size/scale must apply before the next successful frame");
    });
    if (result || f.failure) return 1;
  }
  for (const bool child : {false, true}) {
    Fixture f;
    const int result = f.run([&] {
      const auto target = child ? f.open_child() : f.app.root;
      if (!target) return;
      target->resize_outcomes = {ErrorCode::renderer_recovery_pending, std::nullopt};
      target->on_present = [&] {
        f.app.emit(target, cgpui::WindowResized{.size = {400, 240}, .scale = cgpui::DpiScale{2}});
      };
      f.paint(target);
      f.require(f.errors == 0 && f.after_frames == 1 && f.app.wake_deadline.has_value(),
          "old-frame success discarded a resize queued by its native callback");
      f.app.advance(2);
      f.require(f.app.pump() && f.errors == 0 && f.after_frames == 2 &&
          target->resizes == 2 && target->last_resize_size.width == 400 &&
          target->last_resize_scale.value == 2 && !f.app.wake_deadline,
          "reentrant pending resize must survive old-frame publication and apply before redraw");
    });
    if (result || f.failure) return 2;
  }
  for (const bool child : {false, true}) {
    Fixture f;
    const int result = f.run([&] {
      const auto target = child ? f.open_child() : f.app.root;
      if (!target) return;
      bool resized = false;
      target->resize_outcomes = {ErrorCode::renderer_recovery_pending, std::nullopt};
      f.runtime.set_after_render_callback([&](const auto&, const auto&) {
        if (resized) return;
        resized = true;
        f.app.emit(target, cgpui::WindowResized{.size = {500, 300}, .scale = cgpui::DpiScale{1}});
      });
      f.paint(target);
      f.require(f.errors == 0 && target->begins == 0 && f.after_frames == 0,
          "resize raised during view callbacks must prevent old-extent acquisition");
      f.app.advance(2);
      f.require(f.app.pump() && target->begins == 1 && target->resizes == 2 &&
          f.after_frames == 1 && f.errors == 0, "callback resize did not resume at its applied extent");
    });
    if (result || f.failure) return 3;
  }
  return 0;
}
