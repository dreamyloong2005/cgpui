#include "cgpui/prelude.hpp"
#include "../ui/window_runtime_test_support.hpp"

#include <memory>

namespace {

class CancellationView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::AnimationHandle animation;
  cgpui::AnimationHandle completed;
  cgpui::AnimationHandle context_animation;
  cgpui::AnimationHandle async_animation;
  cgpui::AnimationTransitionHandle transition;
  std::weak_ptr<int> callback_lifetime;
  int callback_count = 0;
  int failure = 0;
  bool context_cancelled = false;
  bool async_cancelled = false;
  bool cancellation_matches = false;
};

cgpui::WindowRuntime* cancellation_runtime = nullptr;
CancellationView* cancellation_view = nullptr;

void dispatch_cancellation() {
  auto& runtime = *cancellation_runtime;
  auto& view = *cancellation_view;
  runtime.advance_time(25);
  const auto before = view.animation.progress();
  const int callbacks_before_cancel = view.callback_count;

  const bool first_cancel = view.animation.cancel();
  const auto cancelled = view.animation.progress();
  const bool transition_cancel = view.transition.cancel();
  const auto transition_snapshot = view.transition.progress();
  const cgpui::RuntimeDiagnosticsSnapshot diagnostics =
      runtime.diagnostics_snapshot();

  runtime.advance_time(200);
  const auto after_time = view.animation.progress();
  const cgpui::RuntimeDiagnosticsSnapshot after_diagnostics =
      runtime.diagnostics_snapshot();
  if (!before.has_value() || !cancelled.has_value() ||
      !after_time.has_value() || !transition_snapshot.has_value()) {
    view.failure = 2;
  } else if (before->elapsed_ms != 25 ||
             cancelled->elapsed_ms != before->elapsed_ms ||
             after_time->elapsed_ms != before->elapsed_ms) {
    view.failure = 3;
  } else if (!cancelled->cancelled || cancelled->complete ||
             !transition_snapshot->animation.cancelled) {
    view.failure = 4;
  } else if (!first_cancel) {
    view.failure = 5;
  } else if (!transition_cancel) {
    view.failure = 6;
  } else if (view.animation.cancel()) {
    view.failure = 7;
  } else if (view.animation.active() || view.animation.complete() ||
             !view.animation.cancelled() || !view.transition.cancelled()) {
    view.failure = 8;
  } else if (!view.callback_lifetime.expired() ||
             view.callback_count != callbacks_before_cancel) {
    view.failure = 9;
  } else if (!view.context_cancelled || !view.async_cancelled ||
             !view.context_animation.cancelled() ||
             !view.async_animation.cancelled()) {
    view.failure = 10;
  } else if (diagnostics.animation_count != 6 ||
             diagnostics.active_animation_count != 0 ||
             diagnostics.completed_animation_count != 2 ||
             diagnostics.cancelled_animation_count != 4) {
    view.failure = 11;
  } else if (!diagnostics.last_animation_cancellation.has_value() ||
             diagnostics.last_animation_cancellation->id !=
                 view.transition.id() ||
             diagnostics.last_animation_cancellation->elapsed_ms != 25 ||
             diagnostics.last_animation_cancellation->duration_ms != 100 ||
             !diagnostics.last_animation_cancellation->timer_was_active ||
             after_diagnostics.cancelled_animation_count != 4) {
    view.failure = 12;
  }
  view.cancellation_matches = view.failure == 0;
}

} // namespace

int main() {
  RuntimeFixture fixture;
  CancellationView view;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  cancellation_runtime = &runtime;
  cancellation_view = &view;
  fixture.app.on_run = &dispatch_cancellation;

  auto callback_lifetime = std::make_shared<int>(1);
  view.callback_lifetime = callback_lifetime;
  view.animation = runtime.start_animation(
      cgpui::AnimationOptions{.duration_ms = 100},
      [&, callback_lifetime](const cgpui::WindowRuntimeContext&,
                             const cgpui::AnimationSnapshot&) {
        view.callback_count += 1;
      });
  callback_lifetime.reset();
  view.completed = runtime.start_animation(
      cgpui::AnimationOptions{},
      [](const cgpui::WindowRuntimeContext&,
         const cgpui::AnimationSnapshot&) {});
  const cgpui::AnimationHandle cancellation_driver = runtime.start_animation(
      cgpui::AnimationOptions{},
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::AnimationSnapshot&) {
        view.context_animation = context.start_animation(
            cgpui::AnimationOptions{.duration_ms = 100},
            [](const cgpui::WindowRuntimeContext&,
               const cgpui::AnimationSnapshot&) {});
        view.async_animation = context.async_context().start_animation(
            cgpui::AnimationOptions{.duration_ms = 100},
            [](const cgpui::WindowRuntimeContext&,
               const cgpui::AnimationSnapshot&) {});
        view.context_cancelled =
            context.cancel_animation(view.context_animation.id());
        view.async_cancelled = context.async_context().cancel_animation(
            view.async_animation.id());
      });
  (void)cancellation_driver;
  view.transition = cgpui::start_animation_transition(
      runtime,
      cgpui::AnimationTransition{.from = 0.0F, .to = 10.0F},
      cgpui::AnimationOptions{.duration_ms = 100},
      [](const cgpui::WindowRuntimeContext&,
         const cgpui::AnimationTransitionSnapshot&) {});

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  cancellation_runtime = nullptr;
  cancellation_view = nullptr;
  if (result != 0) return 1;
  return view.cancellation_matches ? 0 : view.failure;
}
