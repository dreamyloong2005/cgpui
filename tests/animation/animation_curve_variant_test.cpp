#include "cgpui/prelude.hpp"
#include "../ui/window_runtime_test_support.hpp"

#include <cmath>
#include <type_traits>

namespace {

bool near(float left, float right) {
  return std::fabs(left - right) < 0.0001F;
}

static_assert(std::is_trivially_copyable_v<cgpui::AnimationCurve>);
static_assert(sizeof(cgpui::AnimationCurve) <= 40);

int test_legacy_and_pinned_tween_curves() {
  const cgpui::AnimationCurve inherited;
  const cgpui::AnimationCurve quadratic =
      cgpui::AnimationCurve::quadratic();
  const cgpui::AnimationCurve quint =
      cgpui::AnimationCurve::ease_out_quint();
  const cgpui::AnimationCurve bounce =
      cgpui::AnimationCurve::bounce(cgpui::AnimationEasing::ease_in_out);
  const cgpui::AnimationCurve pulse =
      cgpui::AnimationCurve::pulsating_between(0.1F, 0.9F);

  if (!near(inherited.value_at(0.5F, cgpui::AnimationEasing::ease_in),
            0.25F) ||
      !near(quadratic.value_at(0.5F), 0.25F) ||
      !near(quint.value_at(0.5F), 0.96875F) ||
      !near(bounce.value_at(0.25F), 0.5F) ||
      !near(bounce.value_at(0.5F), 1.0F) ||
      !near(bounce.value_at(0.75F), 0.5F) ||
      !near(pulse.value_at(0.0F), 0.5F) ||
      !near(pulse.value_at(0.25F), 0.9F) ||
      !near(pulse.value_at(0.75F), 0.1F)) return 1;
  return 0;
}

int test_parameterized_spring_curve() {
  const cgpui::AnimationCurve spring = cgpui::AnimationCurve::spring(
      cgpui::AnimationSpring{
          .mass = 1.0F,
          .stiffness = 170.0F,
          .damping = 20.0F,
          .initial_velocity = 0.0F,
      });
  const float quarter = spring.value_at(0.25F);
  const float half = spring.value_at(0.5F);
  if (!spring.valid() || !near(spring.value_at(0.0F), 0.0F) ||
      !near(spring.value_at(1.0F), 1.0F) || !std::isfinite(quarter) ||
      !std::isfinite(half) || quarter <= 0.0F || half <= quarter) return 2;

  const cgpui::AnimationCurve invalid = cgpui::AnimationCurve::spring(
      cgpui::AnimationSpring{.mass = 0.0F});
  const cgpui::AnimationCurve critical = cgpui::AnimationCurve::spring(
      cgpui::AnimationSpring{.damping = 26.07681F});
  const cgpui::AnimationCurve overdamped = cgpui::AnimationCurve::spring(
      cgpui::AnimationSpring{.damping = 40.0F});
  if (invalid.valid() || !near(invalid.value_at(0.4F), 0.4F) ||
      !std::isfinite(critical.value_at(0.5F)) ||
      !std::isfinite(overdamped.value_at(0.5F))) return 3;
  return 0;
}

int test_curve_integration_is_shared() {
  const cgpui::AnimationCurve curve =
      cgpui::AnimationCurve::bounce(cgpui::AnimationEasing::ease_in_out);
  cgpui::ElementAnimationStateStore store;
  store.begin_frame(7, 0);
  (void)store.resolve(
      cgpui::ElementKey{.value = "curve"},
      cgpui::AnimationOptions{.duration_ms = 100, .curve = curve});
  (void)store.finish_frame(7);
  store.begin_frame(7, 25);
  const cgpui::ElementAnimationSnapshot snapshot = store.resolve(
      cgpui::ElementKey{.value = "curve"},
      cgpui::AnimationOptions{.duration_ms = 100, .curve = curve});

  cgpui::Style from;
  from.opacity = 0.0F;
  cgpui::Style to;
  to.opacity = 1.0F;
  const cgpui::StyleTween tween{
      .from = from,
      .to = to,
      .curve = curve,
  };
  if (!near(snapshot.linear_progress, 0.25F) ||
      !near(snapshot.eased_progress, 0.5F) ||
      snapshot.curve != curve || !near(tween.value_at(0.25F).opacity, 0.5F)) {
    return 4;
  }
  return 0;
}

class CurveRuntimeView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::AnimationHandle animation;
  cgpui::AnimationCurve curve;
  bool snapshots_match = false;
};

cgpui::WindowRuntime* curve_runtime = nullptr;
CurveRuntimeView* curve_runtime_view = nullptr;

void dispatch_curve_runtime() {
  auto& runtime = *curve_runtime;
  auto& view = *curve_runtime_view;
  const auto start = view.animation.progress();
  runtime.advance_time(25);
  const auto quarter = view.animation.progress();
  runtime.advance_time(75);
  const auto complete = view.animation.progress();
  view.snapshots_match =
      start.has_value() && quarter.has_value() && complete.has_value() &&
      start->curve == view.curve && quarter->curve == view.curve &&
      near(start->eased_progress, 0.0F) &&
      near(quarter->eased_progress, 0.7626953125F) &&
      near(complete->eased_progress, 1.0F) && complete->complete;
}

int test_runtime_uses_curve_variant() {
  RuntimeFixture fixture;
  CurveRuntimeView view;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  view.curve = cgpui::AnimationCurve::ease_out_quint();
  view.animation = runtime.start_animation(
      cgpui::AnimationOptions{.duration_ms = 100, .curve = view.curve},
      [](const cgpui::WindowRuntimeContext&,
         const cgpui::AnimationSnapshot&) {});
  curve_runtime = &runtime;
  curve_runtime_view = &view;
  fixture.app.on_run = &dispatch_curve_runtime;
  const int run_result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  curve_runtime = nullptr;
  curve_runtime_view = nullptr;
  return run_result == 0 && view.snapshots_match ? 0 : 5;
}

} // namespace

int main() {
  if (const int result = test_legacy_and_pinned_tween_curves()) return result;
  if (const int result = test_parameterized_spring_curve()) return result;
  if (const int result = test_curve_integration_is_shared()) return result;
  return test_runtime_uses_curve_variant();
}
