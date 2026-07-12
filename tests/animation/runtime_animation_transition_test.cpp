#include "cgpui/prelude.hpp"
#include "../ui/window_runtime_test_support.hpp"

#include <cmath>
#include <concepts>
#include <memory>
#include <thread>
#include <type_traits>
#include <vector>

namespace {

bool near(float left, float right) {
  return std::fabs(left - right) < 0.0001F;
}

using Handle = cgpui::AnimationTransitionHandle;

static_assert(std::is_move_constructible_v<Handle>);
static_assert(requires(
    cgpui::WindowRuntime& runtime,
    const cgpui::WindowRuntimeContext& context,
    const cgpui::AsyncContextCapability& async,
    cgpui::AnimationTransition transition,
    cgpui::AnimationOptions options,
    cgpui::AnimationTransitionCallback callback) {
  { cgpui::start_animation_transition(
        runtime, transition, options, callback) } -> std::same_as<Handle>;
  { cgpui::start_animation_transition(
        context, transition, options, callback) } -> std::same_as<Handle>;
  { cgpui::start_animation_transition(
        async, transition, options, callback) } -> std::same_as<Handle>;
});

class TransitionView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  Handle transition;
  Handle context_immediate;
  Handle async_immediate;
  std::thread::id runtime_thread;
  std::vector<float> callback_values;
  bool callbacks_on_runtime_thread = true;
  bool snapshots_match = false;
  bool immediate_transitions_match = false;
  bool immediate_callback_released = false;
  bool invalid_callback_rejected = false;
};

cgpui::WindowRuntime* transition_runtime = nullptr;
TransitionView* transition_view = nullptr;

void dispatch_transition_sequence() {
  auto& runtime = *transition_runtime;
  auto& view = *transition_view;
  view.runtime_thread = std::this_thread::get_id();

  const auto start = view.transition.progress();
  const bool start_matches =
      start.has_value() && !start->animation.complete &&
      start->animation.linear_progress == 0.0F && near(start->value, 10.0F);

  runtime.advance_time(25);
  const auto quarter = view.transition.progress();
  const bool quarter_matches =
      quarter.has_value() && !quarter->animation.complete &&
      near(quarter->animation.eased_progress, 0.4375F) &&
      near(quarter->value, 14.375F);

  runtime.advance_time(25);
  const auto half = view.transition.progress();
  const bool half_matches =
      half.has_value() && !half->animation.complete &&
      near(half->value, 17.5F);

  runtime.advance_time(50);
  const auto complete = view.transition.progress();
  const bool complete_matches =
      complete.has_value() && complete->animation.complete &&
      near(complete->value, 20.0F) && !view.transition.active() &&
      view.transition.complete() && !view.transition.cancel();

  const Handle invalid = cgpui::start_animation_transition(
      runtime,
      cgpui::AnimationTransition{.from = 3.0F, .to = 4.0F},
      cgpui::AnimationOptions{.duration_ms = 10},
      {});
  view.invalid_callback_rejected = invalid.id().value == 0;
  view.snapshots_match =
      start_matches && quarter_matches && half_matches && complete_matches &&
      view.callback_values.size() == 3 &&
      near(view.callback_values[0], 14.375F) &&
      near(view.callback_values[1], 17.5F) &&
      near(view.callback_values[2], 20.0F);
}

} // namespace

int main() {
  RuntimeFixture fixture;
  TransitionView view;
  fixture.app.on_run = &dispatch_transition_sequence;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  transition_runtime = &runtime;
  transition_view = &view;

  view.transition = cgpui::start_animation_transition(
      runtime,
      cgpui::AnimationTransition{.from = 10.0F, .to = 20.0F},
      cgpui::AnimationOptions{
          .duration_ms = 100,
          .easing = cgpui::AnimationEasing::ease_out,
          .tick_interval_ms = 16,
      },
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::AnimationTransitionSnapshot& snapshot) {
        view.callbacks_on_runtime_thread =
            view.callbacks_on_runtime_thread &&
            std::this_thread::get_id() == view.runtime_thread;
        view.callback_values.push_back(snapshot.value);
        if (view.callback_values.size() == 1) {
          float context_value = -1.0F;
          float async_value = -1.0F;
          auto callback_lifetime = std::make_shared<int>(1);
          const std::weak_ptr<int> callback_lifetime_weak = callback_lifetime;
          view.context_immediate = cgpui::start_animation_transition(
              context,
              cgpui::AnimationTransition{.from = 1.0F, .to = 2.0F},
              cgpui::AnimationOptions{},
              [&, callback_lifetime](const cgpui::WindowRuntimeContext&,
                  const cgpui::AnimationTransitionSnapshot& immediate) {
                context_value = immediate.value;
              });
          callback_lifetime.reset();
          view.async_immediate = cgpui::start_animation_transition(
              context.async_context(),
              cgpui::AnimationTransition{.from = 5.0F, .to = 8.0F},
              cgpui::AnimationOptions{},
              [&](const cgpui::WindowRuntimeContext&,
                  const cgpui::AnimationTransitionSnapshot& immediate) {
                async_value = immediate.value;
              });
          view.immediate_transitions_match =
              near(context_value, 2.0F) && near(async_value, 8.0F) &&
              view.context_immediate.complete() &&
              view.async_immediate.complete();
          view.immediate_callback_released = callback_lifetime_weak.expired();
        }
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  transition_runtime = nullptr;
  transition_view = nullptr;
  if (result != 0) return 1;
  if (!view.callbacks_on_runtime_thread) return 2;
  if (!view.snapshots_match) return 3;
  if (!view.immediate_transitions_match) return 4;
  if (!view.immediate_callback_released) return 5;
  if (!view.invalid_callback_rejected) return 6;
  return 0;
}
