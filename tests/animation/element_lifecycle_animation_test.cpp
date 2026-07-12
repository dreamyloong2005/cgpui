#include "cgpui/prelude.hpp"
#include "../ui/window_runtime_test_support.hpp"

#include <cmath>
#include <concepts>
#include <vector>

namespace {

bool near(float left, float right) {
  return std::fabs(left - right) < 0.0001F;
}

static_assert(requires(
    cgpui::ElementAnimationStateStore& store,
    cgpui::ElementKey key,
    cgpui::AnimationOptions options,
    cgpui::AnyElement child,
    cgpui::ElementAnimationCallback callback) {
  store.begin_frame(std::uint64_t{}, std::uint64_t{});
  { store.resolve(key, options) } ->
      std::same_as<cgpui::ElementAnimationSnapshot>;
  { store.finish_frame(std::uint64_t{}) } ->
      std::same_as<cgpui::ElementAnimationFrameResult>;
  { cgpui::with_animation(
        key, options, std::move(child), callback) } ->
      std::same_as<cgpui::AnyElement>;
});

int test_scoped_store_mount_update_unmount_and_isolation() {
  cgpui::ElementAnimationStateStore store;
  const cgpui::ElementKey key{.value = "pulse"};
  const cgpui::AnimationOptions options{
      .duration_ms = 100,
      .easing = cgpui::AnimationEasing::ease_in_out,
  };

  store.begin_frame(1, 0);
  const cgpui::ElementAnimationSnapshot mounted = store.resolve(key, options);
  const cgpui::ElementAnimationFrameResult first = store.finish_frame(1);
  if (!mounted.mounted || mounted.complete || mounted.scope_id != 1 ||
      mounted.key != key || !near(mounted.eased_progress, 0.0F) ||
      first.active_count != 1 || first.completed_count != 0 ||
      first.unmounted_count != 0 || !first.requests_next_frame()) return 1;

  store.begin_frame(1, 50);
  const cgpui::ElementAnimationSnapshot updated = store.resolve(key, options);
  const cgpui::ElementAnimationFrameResult second = store.finish_frame(1);
  if (updated.mounted || updated.complete || updated.elapsed_ms != 50 ||
      !near(updated.linear_progress, 0.5F) ||
      !near(updated.eased_progress, 0.5F) || second.active_count != 1) return 2;

  store.begin_frame(2, 50);
  const cgpui::ElementAnimationSnapshot isolated = store.resolve(key, options);
  const cgpui::ElementAnimationFrameResult other_scope = store.finish_frame(2);
  if (!isolated.mounted || isolated.scope_id != 2 ||
      !near(isolated.linear_progress, 0.0F) || other_scope.active_count != 1 ||
      store.size() != 2) return 3;

  store.begin_frame(1, 75);
  const cgpui::ElementAnimationFrameResult unmounted = store.finish_frame(1);
  if (unmounted.unmounted_count != 1 || unmounted.active_count != 0 ||
      store.contains(1, key) || !store.contains(2, key)) return 4;

  store.begin_frame(2, 150);
  const cgpui::ElementAnimationSnapshot completed = store.resolve(key, options);
  const cgpui::ElementAnimationFrameResult final = store.finish_frame(2);
  if (!completed.complete || !near(completed.eased_progress, 1.0F) ||
      final.active_count != 0 || final.completed_count != 1 ||
      final.requests_next_frame()) return 5;
  return 0;
}

class LifecycleAnimationView final : public cgpui::View {
 public:
  cgpui::AnyElement render(cgpui::ViewContext&) override {
    ++render_count;
    if (!show_animation) return {};
    return cgpui::with_animation(
        cgpui::ElementKey{.value = "runtime-pulse"},
        cgpui::AnimationOptions{
            .duration_ms = 100,
            .easing = cgpui::AnimationEasing::linear,
            .tick_interval_ms = 16,
        },
        cgpui::into_element(cgpui::div().size(20.0F, 10.0F)),
        [&](cgpui::Element&,
            const cgpui::ElementAnimationSnapshot& snapshot) {
          snapshots.push_back(snapshot);
        });
  }

  void paint(cgpui::PaintList&, cgpui::Size) override {
    ++paint_count;
  }

  bool show_animation = true;
  int render_count = 0;
  int paint_count = 0;
  std::vector<cgpui::ElementAnimationSnapshot> snapshots;
};

RuntimeFixture* lifecycle_fixture = nullptr;
cgpui::WindowRuntime* lifecycle_runtime = nullptr;
LifecycleAnimationView* lifecycle_view = nullptr;

void dispatch_lifecycle_animation_sequence() {
  auto& fixture = *lifecycle_fixture;
  auto& runtime = *lifecycle_runtime;
  auto& view = *lifecycle_view;
  if (view.snapshots.size() != 1 || !view.snapshots[0].mounted ||
      !fixture.app.delayed_wakeup_pending) return;

  fixture.app.advance_monotonic_time(50);
  fixture.app.dispatch_delayed_wakeup();
  fixture.app.advance_monotonic_time(50);
  fixture.app.dispatch_delayed_wakeup();
  view.show_animation = false;
  runtime.request_render();
}

int test_runtime_preserves_animation_across_rendered_wrapper_instances() {
  RuntimeFixture fixture;
  LifecycleAnimationView view;
  lifecycle_fixture = &fixture;
  lifecycle_runtime = nullptr;
  lifecycle_view = &view;
  fixture.app.on_run = &dispatch_lifecycle_animation_sequence;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  lifecycle_runtime = &runtime;
  const int result = runtime.run(cgpui::WindowDescriptor{});
  lifecycle_fixture = nullptr;
  lifecycle_runtime = nullptr;
  lifecycle_view = nullptr;

  if (result != 0 || view.snapshots.size() != 3 ||
      !view.snapshots[0].mounted || view.snapshots[1].mounted ||
      view.snapshots[2].mounted || view.snapshots[0].complete ||
      view.snapshots[1].complete || !view.snapshots[2].complete ||
      !near(view.snapshots[0].linear_progress, 0.0F) ||
      !near(view.snapshots[1].linear_progress, 0.5F) ||
      !near(view.snapshots[2].linear_progress, 1.0F)) return 10;
  if (view.render_count != 4 || view.paint_count != 4 ||
      fixture.renderer.begin_frame_count != 4 ||
      fixture.window.request_redraw_count != 4 ||
      fixture.app.delayed_wakeup_delays != std::vector<std::uint64_t>({16, 16}) ||
      fixture.app.delayed_wakeup_pending) return 11;
  return 0;
}

} // namespace

int main() {
  if (const int result =
          test_scoped_store_mount_update_unmount_and_isolation();
      result != 0) return result;
  return test_runtime_preserves_animation_across_rendered_wrapper_instances();
}
