#include "window_runtime_test_support.hpp"

namespace {

struct ClosureEntity {
  int value = 0;
};

int test_removed_view_cleans_observation_and_subscription_state() {
  RuntimeFixture fixture;
  RegistryView child_view;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const cgpui::ViewId child_view_id = runtime.register_view(child_view);
  const cgpui::EntityId<ClosureEntity> entity_id =
      runtime.insert_entity(ClosureEntity{.value = 1});

  runtime.subscribe_view_to_entity(child_view_id, entity_id);
  cgpui::Subscription view_subscription =
      runtime.observe_view_subscription(
          child_view_id,
          [](const cgpui::WindowRuntimeContext&, cgpui::ViewId) {});

  if (!view_subscription.connected()) {
    return 1;
  }
  if (runtime.subscriptions_for_view(child_view_id).size() != 1) {
    return 2;
  }

  const cgpui::RuntimeDiagnosticsSnapshot before =
      runtime.diagnostics_snapshot();
  if (before.view_entity_subscription_count != 1 ||
      before.view_observer_count != 1 ||
      before.connected_subscription_count != 1) {
    return 3;
  }

  if (!runtime.remove_view(child_view_id)) {
    return 4;
  }
  if (runtime.find_view(child_view_id) != nullptr ||
      runtime.is_view_id_allocated(child_view_id)) {
    return 5;
  }
  if (!runtime.subscriptions_for_view(child_view_id).empty()) {
    return 6;
  }

  const cgpui::RuntimeDiagnosticsSnapshot after =
      runtime.diagnostics_snapshot();
  if (after.view_entity_subscription_count != 0 ||
      after.view_observer_count != 0 ||
      after.connected_subscription_count != 0) {
    return 7;
  }
  if (view_subscription.connected() || view_subscription.release()) {
    return 8;
  }

  return 0;
}

} // namespace

int main() {
  return test_removed_view_cleans_observation_and_subscription_state();
}
