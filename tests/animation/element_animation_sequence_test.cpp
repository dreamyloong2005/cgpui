#include "cgpui/prelude.hpp"
#include "../ui/window_runtime_test_support.hpp"

#include <cmath>
#include <concepts>
#include <span>
#include <vector>

namespace {

bool near(float left, float right) {
  return std::fabs(left - right) < 0.0001F;
}

static_assert(requires(
    cgpui::ElementAnimationStage stage,
    cgpui::ElementAnimationStateStore& store,
    cgpui::ElementKey key,
    std::span<const cgpui::ElementAnimationStage> stages,
    cgpui::AnyElement child,
    cgpui::ElementAnimationCallback callback) {
  { stage.repeat() } -> std::same_as<cgpui::ElementAnimationStage>;
  { store.resolve(key, stages) } ->
      std::same_as<cgpui::ElementAnimationSnapshot>;
  { cgpui::with_animations(
        key, std::vector<cgpui::ElementAnimationStage>{},
        std::move(child), callback) } -> std::same_as<cgpui::AnyElement>;
});

int test_repeat_wraps_progress_and_never_advances() {
  cgpui::ElementAnimationStateStore store;
  const cgpui::ElementKey key{.value = "repeat"};
  const std::vector stages{
      cgpui::ElementAnimationStage{
          .animation = cgpui::AnimationOptions{.duration_ms = 100},
      }.repeat(),
      cgpui::ElementAnimationStage{
          .animation = cgpui::AnimationOptions{.duration_ms = 50},
      },
  };

  store.begin_frame(1, 0);
  const auto start = store.resolve(key, std::span{stages});
  (void)store.finish_frame(1);
  store.begin_frame(1, 100);
  const auto boundary = store.resolve(key, std::span{stages});
  (void)store.finish_frame(1);
  store.begin_frame(1, 125);
  const auto wrapped = store.resolve(key, std::span{stages});
  const auto frame = store.finish_frame(1);
  if (!start.mounted || start.stage_index != 0 || start.iteration != 0 ||
      !near(start.linear_progress, 0.0F) || boundary.stage_index != 0 ||
      boundary.iteration != 0 || !near(boundary.linear_progress, 1.0F) ||
      wrapped.stage_index != 0 || wrapped.iteration != 1 ||
      !wrapped.repeating || !near(wrapped.linear_progress, 0.25F) ||
      wrapped.complete || frame.active_count != 1) return 1;
  return 0;
}

int test_chain_delivers_stage_final_before_advancing() {
  cgpui::ElementAnimationStateStore store;
  const cgpui::ElementKey key{.value = "chain"};
  const std::vector stages{
      cgpui::ElementAnimationStage{
          .animation = cgpui::AnimationOptions{.duration_ms = 100},
      },
      cgpui::ElementAnimationStage{
          .animation = cgpui::AnimationOptions{
              .duration_ms = 50,
              .easing = cgpui::AnimationEasing::ease_in_out,
          },
      },
  };

  store.begin_frame(2, 0);
  (void)store.resolve(key, std::span{stages});
  (void)store.finish_frame(2);
  store.begin_frame(2, 101);
  const auto first_final = store.resolve(key, std::span{stages});
  const auto advanced = store.finish_frame(2);
  store.begin_frame(2, 126);
  const auto second_half = store.resolve(key, std::span{stages});
  (void)store.finish_frame(2);
  store.begin_frame(2, 152);
  const auto second_final = store.resolve(key, std::span{stages});
  const auto finished = store.finish_frame(2);
  if (first_final.stage_index != 0 || first_final.complete ||
      !near(first_final.linear_progress, 1.0F) || advanced.active_count != 1 ||
      second_half.stage_index != 1 ||
      !near(second_half.eased_progress, 0.5F) || second_final.stage_index != 1 ||
      !second_final.complete || !near(second_final.linear_progress, 1.0F) ||
      finished.active_count != 0 || finished.completed_count != 1) return 2;
  return 0;
}

int test_invalid_sequences_preserve_the_child() {
  auto empty_child = cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  cgpui::Element* empty_child_pointer = empty_child.get();
  cgpui::AnyElement empty_result = cgpui::with_animations(
      cgpui::ElementKey{.value = "empty"}, {}, std::move(empty_child),
      [](cgpui::Element&, const cgpui::ElementAnimationSnapshot&) {});

  auto zero_repeat_child =
      cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  cgpui::Element* zero_repeat_pointer = zero_repeat_child.get();
  cgpui::AnyElement zero_repeat_result = cgpui::with_animations(
      cgpui::ElementKey{.value = "zero-repeat"},
      {cgpui::ElementAnimationStage{
           .animation = cgpui::AnimationOptions{.duration_ms = 0},
       }.repeat()},
      std::move(zero_repeat_child),
      [](cgpui::Element&, const cgpui::ElementAnimationSnapshot&) {});
  return empty_result.get() == empty_child_pointer &&
                 zero_repeat_result.get() == zero_repeat_pointer
             ? 0
             : 3;
}

class SequenceView final : public cgpui::View {
 public:
  cgpui::AnyElement render(cgpui::ViewContext&) override {
    ++render_count;
    return cgpui::with_animations(
        cgpui::ElementKey{.value = "runtime-chain"},
        {
            cgpui::ElementAnimationStage{
                .animation = cgpui::AnimationOptions{.duration_ms = 50},
            },
            cgpui::ElementAnimationStage{
                .animation = cgpui::AnimationOptions{.duration_ms = 50},
            },
        },
        cgpui::into_element(cgpui::div().size(20.0F, 10.0F)),
        [&](cgpui::Element&, const cgpui::ElementAnimationSnapshot& snapshot) {
          snapshots.push_back(snapshot);
        });
  }

  void paint(cgpui::PaintList&, cgpui::Size) override {}

  int render_count = 0;
  std::vector<cgpui::ElementAnimationSnapshot> snapshots;
};

RuntimeFixture* sequence_fixture = nullptr;
SequenceView* sequence_view = nullptr;

void dispatch_sequence() {
  if (sequence_view->snapshots.size() != 1) return;
  sequence_fixture->app.advance_monotonic_time(51);
  sequence_fixture->app.dispatch_delayed_wakeup();
  sequence_fixture->app.advance_monotonic_time(25);
  sequence_fixture->app.dispatch_delayed_wakeup();
  sequence_fixture->app.advance_monotonic_time(26);
  sequence_fixture->app.dispatch_delayed_wakeup();
}

int test_runtime_chain_survives_wrapper_reconstruction() {
  RuntimeFixture fixture;
  SequenceView view;
  sequence_fixture = &fixture;
  sequence_view = &view;
  fixture.app.on_run = &dispatch_sequence;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  const int result = runtime.run(cgpui::WindowDescriptor{});
  sequence_fixture = nullptr;
  sequence_view = nullptr;
  if (result != 0 || view.snapshots.size() != 4 || view.render_count != 4 ||
      view.snapshots[0].stage_index != 0 ||
      view.snapshots[1].stage_index != 0 ||
      view.snapshots[2].stage_index != 1 ||
      view.snapshots[3].stage_index != 1 ||
      !near(view.snapshots[0].linear_progress, 0.0F) ||
      !near(view.snapshots[1].linear_progress, 1.0F) ||
      !near(view.snapshots[2].linear_progress, 0.5F) ||
      !view.snapshots[3].complete ||
      fixture.app.delayed_wakeup_delays !=
          std::vector<std::uint64_t>({16, 13, 4}) ||
      fixture.app.delayed_wakeup_pending) return 4;
  return 0;
}

} // namespace

int main() {
  if (const int result = test_repeat_wraps_progress_and_never_advances())
    return result;
  if (const int result = test_chain_delivers_stage_final_before_advancing())
    return result;
  if (const int result = test_invalid_sequences_preserve_the_child())
    return result;
  return test_runtime_chain_survives_wrapper_reconstruction();
}
