#include "baseline_measure.hpp"
#include "baseline_workloads.hpp"

#include "cgpui/prelude.hpp"
#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/test_app.hpp"

#include <algorithm>
#include <array>
#include <memory>

namespace performance_baseline {
namespace {
class SchedulingView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
  cgpui::EventResult handle_event(const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
    if (key == nullptr) return cgpui::EventResult::unhandled();
    if (key->key_code == 'A') {
      for (std::size_t index = 0; index < 64; ++index) {
        tasks.push_back(context.spawn_task(
            [this, index](const auto&) { ++completions[index]; }));
      }
      return cgpui::EventResult::consumed_event();
    }
    if (key->key_code == 'P') {
      animation = context.start_animation({.duration_ms = 160, .tick_interval_ms = 16},
          [this](const auto&, const cgpui::AnimationSnapshot& snapshot) {
            ticks.push_back(snapshot.elapsed_ms);
          });
      return cgpui::EventResult::consumed_event();
    }
    return cgpui::EventResult::unhandled();
  }
  std::vector<cgpui::TaskHandle> tasks;
  std::array<std::size_t, 64> completions{};
  cgpui::AnimationHandle animation;
  std::vector<std::uint64_t> ticks;
};
} // namespace

void append_scheduling_samples(std::vector<Sample>& samples) {
  cgpui::TestApp app;
  auto owned = std::make_unique<SchedulingView>();
  auto& view = *owned;
  auto window = app.open_window({}, std::move(owned));
  bool accepted = false;
  std::size_t completed_requests = 0;
  samples.push_back(measure("async_wakeup", 64, [&] {
    accepted = window.dispatch_platform_event(cgpui::KeyboardKey{.key_code = 'A'});
    for (const auto& task : view.tasks) {
      if (app.complete_task(task.id())) ++completed_requests;
    }
    app.drain_task_completions();
  }, [&] {
    const auto delivered = std::ranges::count_if(view.completions,
        [](std::size_t count) { return count == 1; });
    return WorkloadObservation{static_cast<std::size_t>(delivered),
        accepted && view.tasks.size() == 64 && completed_requests == 64 &&
        std::ranges::all_of(view.tasks, [](const auto& task) { return task.complete(); })};
  }));

  samples.push_back(measure("frame_pacing", 10, [&] {
    accepted = window.dispatch_platform_event(cgpui::KeyboardKey{.key_code = 'P'});
    for (int frame = 0; frame < 10; ++frame) app.advance_time_until_parked(16);
  }, [&] {
    const std::vector<std::uint64_t> expected{16, 32, 48, 64, 80, 96, 112, 128, 144, 160};
    return WorkloadObservation{view.ticks.size(),
        accepted && view.animation.complete() && view.ticks == expected};
  }));
}

} // namespace performance_baseline
