#include "window_runtime_test_support.hpp"

namespace {
RuntimeFixture* entity_context_fixture = nullptr;

void dispatch_entity_context_sequence() {
  auto& callback = entity_context_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 70,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 71,
      .action = cgpui::KeyAction::pressed});
}

int test_context_can_access_runtime_entities() {
  RuntimeFixture fixture;
  entity_context_fixture = &fixture;
  fixture.app.on_run = &dispatch_entity_context_sequence;
  fixture.view.exercise_entity_context_access = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  entity_context_fixture = nullptr;

  if (result != 0) {
    return 112;
  }
  if (fixture.view.keyboard_key_count != 2) {
    return 113;
  }
  if (fixture.view.inserted_entity_id.value == 0 ||
      fixture.view.first_entity_read_value != 10 ||
      fixture.view.second_entity_read_value != 21) {
    return 114;
  }
  if (!fixture.view.removed_entity ||
      fixture.view.removed_entity_again ||
      !fixture.view.missing_entity_after_remove) {
    return 115;
  }
  if (fixture.view.emplaced_entity_id.value <=
          fixture.view.inserted_entity_id.value ||
      fixture.view.emplaced_entity_read_value != 42) {
    return 116;
  }

  return 0;
}

RuntimeFixture* view_model_subscription_fixture = nullptr;

void dispatch_view_model_subscription_sequence() {
  auto& callback = view_model_subscription_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 77,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_tracks_view_model_subscriptions() {
  RuntimeFixture fixture;
  view_model_subscription_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_model_subscription_sequence;
  fixture.view.exercise_view_model_subscriptions = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  view_model_subscription_fixture = nullptr;

  if (result != 0) {
    return 188;
  }
  if (fixture.view.subscriptions_after_subscribe != 1 ||
      !fixture.view.first_subscription_matches_entity) {
    return 189;
  }
  if (!fixture.view.notified_subscribed_entity ||
      fixture.view.notified_missing_entity) {
    return 190;
  }
  if (!fixture.view.invalidation_after_subscribed_notify.render ||
      !fixture.view.invalidation_after_subscribed_notify.layout ||
      !fixture.view.invalidation_after_subscribed_notify.paint) {
    return 191;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1) {
    return 192;
  }

  return 0;
}

RuntimeFixture* view_context_model_helper_fixture = nullptr;

void dispatch_view_context_model_helper_sequence() {
  auto& callback = view_context_model_helper_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 78,
      .action = cgpui::KeyAction::pressed});
}

int test_view_context_model_helpers_create_read_update_and_remove() {
  RuntimeFixture fixture;
  view_context_model_helper_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_context_model_helper_sequence;
  fixture.view.exercise_view_context_model_helpers = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  view_context_model_helper_fixture = nullptr;

  if (result != 0) {
    return 254;
  }
  if (fixture.view.model_id.value == 0 ||
      fixture.view.first_model_read_value != 10) {
    return 255;
  }
  if (!fixture.view.context_alias_same_view_context_type) {
    return 261;
  }
  if (!fixture.view.updated_model ||
      fixture.view.updated_model_read_value != 24 ||
      fixture.view.update_missing_model) {
    return 256;
  }
  if (!fixture.view.invalidation_after_model_update.render ||
      !fixture.view.invalidation_after_model_update.layout ||
      !fixture.view.invalidation_after_model_update.paint) {
    return 257;
  }
  if (!fixture.view.removed_model ||
      fixture.view.removed_model_again ||
      !fixture.view.missing_model_after_remove) {
    return 258;
  }
  if (!fixture.view.invalidation_after_model_remove.render ||
      !fixture.view.invalidation_after_model_remove.layout ||
      !fixture.view.invalidation_after_model_remove.paint) {
    return 259;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1) {
    return 260;
  }

  return 0;
}

RuntimeFixture* view_context_model_observe_fixture = nullptr;

void dispatch_view_context_model_observe_sequence() {
  auto& callback = view_context_model_observe_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 79,
      .action = cgpui::KeyAction::pressed});
}

int test_view_context_observes_model_changes() {
  RuntimeFixture fixture;
  view_context_model_observe_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_context_model_observe_sequence;
  fixture.view.exercise_view_context_model_observe_helper = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  view_context_model_observe_fixture = nullptr;

  if (result != 0) {
    return 266;
  }
  if (!fixture.view.observed_model ||
      fixture.view.observed_missing_model ||
      fixture.view.missing_model_observer_count != 0) {
    return 267;
  }
  if (!fixture.view.updated_model ||
      !fixture.view.removed_model ||
      fixture.view.update_missing_model) {
    return 268;
  }
  if (fixture.view.observer_count_after_update != 1 ||
      fixture.view.observer_value_after_update != 13) {
    return 269;
  }
  if (fixture.view.observer_count_after_remove != 2 ||
      fixture.view.observer_value_after_remove != -1) {
    return 270;
  }
  if (fixture.view.model_observer_saw_view_id != fixture.view.first_view_id ||
      fixture.view.model_observer_saw_model != fixture.view.model_id) {
    return 271;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1) {
    return 272;
  }

  return 0;
}

RuntimeFixture* subscription_ownership_fixture = nullptr;

void dispatch_subscription_ownership_sequence() {
  auto& callback = subscription_ownership_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 83,
      .action = cgpui::KeyAction::pressed});
}

int test_subscription_token_disconnects_observers_on_drop_and_removal() {
  RuntimeFixture fixture;
  subscription_ownership_fixture = &fixture;
  fixture.app.on_run = &dispatch_subscription_ownership_sequence;
  fixture.view.exercise_subscription_ownership_token = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  subscription_ownership_fixture = nullptr;

  if (result != 0) {
    return 273;
  }
  if (!fixture.view.observed_model ||
      !fixture.view.subscription_id_was_valid ||
      fixture.view.owned_subscription_id.value == 0) {
    return 274;
  }
  if (!fixture.view.updated_model ||
      fixture.view.observer_count_after_update != 1) {
    return 275;
  }
  if (fixture.view.notified_after_token_drop ||
      fixture.view.observer_count_after_drop != 1) {
    return 276;
  }
  if (fixture.view.removed_subscription_id.value == 0 ||
      !fixture.view.removed_subscription_disconnect ||
      fixture.view.removed_subscription_duplicate_disconnect ||
      fixture.view.removed_subscription_release_after_remove ||
      !fixture.view.missing_subscription_release) {
    return 277;
  }
  if (fixture.view.notified_after_remove ||
      fixture.view.observer_count_after_remove != 1) {
    return 278;
  }
  if (fixture.view.moved_subscription_id.value == 0 ||
      !fixture.view.moved_subscription_connected_before_move ||
      fixture.view.moved_subscription_source_connected_after_move ||
      fixture.view.moved_subscription_source_release_after_move ||
      !fixture.view.moved_subscription_target_connected_after_move ||
      !fixture.view.moved_subscription_release ||
      fixture.view.moved_subscription_connected_after_release ||
      fixture.view.moved_subscription_duplicate_release) {
    return 398;
  }
  if (fixture.view.notified_after_move_release ||
      fixture.view.observer_count_after_move_release != 1) {
    return 399;
  }
  const cgpui::RuntimeDiagnosticsSnapshot diagnostics =
      runtime.diagnostics_snapshot();
  if (diagnostics.entity_observer_count != 0 ||
      diagnostics.connected_subscription_count != 0) {
    return 400;
  }

  return 0;
}

RuntimeFixture* weak_handle_fixture = nullptr;

void dispatch_weak_handle_sequence() {
  auto& callback = weak_handle_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 87,
      .action = cgpui::KeyAction::pressed});
}

int test_weak_entity_and_view_handles_upgrade_softly() {
  RuntimeFixture fixture;
  weak_handle_fixture = &fixture;
  fixture.app.on_run = &dispatch_weak_handle_sequence;
  fixture.view.exercise_weak_entity_and_view_handles = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  weak_handle_fixture = nullptr;

  if (result != 0) {
    return 260;
  }
  if (fixture.view.weak_model.empty() ||
      fixture.view.weak_model.id() != fixture.view.model_id) {
    return 261;
  }
  if (!fixture.view.upgraded_weak_model.has_value() ||
      *fixture.view.upgraded_weak_model != fixture.view.model_id ||
      fixture.view.weak_model_read_value != 41) {
    return 262;
  }
  if (!fixture.view.removed_model ||
      fixture.view.upgraded_removed_weak_model.has_value()) {
    return 263;
  }
  if (fixture.view.weak_view.empty() ||
      fixture.view.weak_view.id() != fixture.view.first_allocated_view_id) {
    return 264;
  }
  if (!fixture.view.upgraded_weak_view.has_value() ||
      *fixture.view.upgraded_weak_view != fixture.view.first_allocated_view_id ||
      !fixture.view.upgraded_root_weak_view.has_value() ||
      *fixture.view.upgraded_root_weak_view != fixture.view.first_view_id ||
      fixture.view.upgraded_missing_weak_view.has_value()) {
    return 265;
  }

  return 0;
}

RuntimeFixture* entity_handle_fixture = nullptr;

void dispatch_entity_handle_sequence() {
  auto& callback = entity_handle_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 72,
      .action = cgpui::KeyAction::pressed});
}

RuntimeFixture* cross_runtime_entity_fixture = nullptr;

void dispatch_cross_runtime_entity_sequence() {
  auto& callback = cross_runtime_entity_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 67,
      .action = cgpui::KeyAction::pressed});
}

class CrossRuntimeEntityBoundaryView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    if (create_exported_entity) {
      exported_entity = context.new_entity<RuntimeEntity>(41);
      const RuntimeEntity* exported = exported_entity.read(context);
      exported_entity_read_value =
          exported == nullptr ? -1 : exported->value;
    }

    if (exercise_imported_entity) {
      local_entity = context.new_entity<RuntimeEntity>(99);
      same_numeric_entity_id = local_entity.id() == imported_entity.id();
      imported_read = imported_entity.read(context) != nullptr;
      imported_update = imported_entity.update(
          context,
          [](RuntimeEntity& entity) {
            entity.value = 123;
          });
      imported_invalidate = imported_entity.invalidate(context);
      imported_remove = imported_entity.remove(context);
      imported_weak_upgrade =
          imported_entity.downgrade().upgrade(context).has_value();

      const RuntimeEntity* local = local_entity.read(context);
      local_entity_still_present = local != nullptr;
      local_entity_value = local == nullptr ? -1 : local->value;
    }

    return cgpui::EventResult::consumed_event();
  }

  bool create_exported_entity = false;
  bool exercise_imported_entity = false;
  cgpui::EntityHandle<RuntimeEntity> imported_entity{};
  cgpui::EntityHandle<RuntimeEntity> exported_entity{};
  cgpui::EntityHandle<RuntimeEntity> local_entity{};
  int exported_entity_read_value = -1;
  bool same_numeric_entity_id = false;
  bool imported_read = true;
  bool imported_update = true;
  bool imported_invalidate = true;
  bool imported_remove = true;
  bool imported_weak_upgrade = true;
  bool local_entity_still_present = false;
  int local_entity_value = -1;
};

int test_entity_handle_read_update_and_downgrade() {
  RuntimeFixture fixture;
  entity_handle_fixture = &fixture;
  fixture.app.on_run = &dispatch_entity_handle_sequence;
  fixture.view.exercise_entity_handle_helpers = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  entity_handle_fixture = nullptr;

  if (result != 0) {
    return 278;
  }
  if (fixture.view.entity_handle.empty() ||
      fixture.view.entity_handle_read_value != 55) {
    return 279;
  }
  if (!fixture.view.entity_handle_update ||
      fixture.view.entity_handle_updated_value != 89 ||
      fixture.view.entity_handle_missing_update) {
    return 280;
  }
  if (!fixture.view.entity_handle_observed) {
    return 284;
  }
  if (!fixture.view.entity_handle_subscription_connected) {
    return 285;
  }
  if (fixture.view.entity_handle_observer_count_after_update != 2) {
    return 286;
  }
  if (fixture.view.entity_handle_observer_value_sum_after_update != 178) {
    return 287;
  }
  if (!fixture.view.entity_handle_transaction_result.has_value() ||
      *fixture.view.entity_handle_transaction_result != 100 ||
      !fixture.view.entity_handle_context_transaction ||
      !fixture.view.entity_handle_transaction_context_matched ||
      fixture.view.entity_handle_transaction_value != 100) {
    return 288;
  }
  if (fixture.view.entity_handle_observer_count_after_transaction != 6 ||
      fixture.view.entity_handle_observer_value_sum_after_transaction != 576) {
    return 289;
  }
  if (!fixture.view.invalidation_after_model_update.render ||
      !fixture.view.invalidation_after_model_update.layout ||
      !fixture.view.invalidation_after_model_update.paint) {
    return 281;
  }
  if (!fixture.view.removed_entity || !fixture.view.entity_handle_missing_read ||
      fixture.view.upgraded_removed_weak_model.has_value()) {
    return 282;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1) {
    return 283;
  }
  return 0;
}

int test_entity_handle_and_context_invalidate_entities() {
  RuntimeFixture fixture;
  entity_handle_fixture = &fixture;
  fixture.app.on_run = &dispatch_entity_handle_sequence;
  fixture.view.exercise_entity_invalidation_helpers = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  entity_handle_fixture = nullptr;

  if (result != 0) {
    return 290;
  }
  if (fixture.view.entity_handle.empty() ||
      !fixture.view.entity_handle_observed) {
    return 291;
  }
  if (fixture.view.missing_entity_invalidate ||
      fixture.view.empty_entity_invalidate ||
      fixture.view.invalidation_after_missing_entity_invalidate.render ||
      fixture.view.invalidation_after_missing_entity_invalidate.layout ||
      fixture.view.invalidation_after_missing_entity_invalidate.paint) {
    return 292;
  }
  if (!fixture.view.entity_handle_invalidate ||
      !fixture.view.invalidation_after_entity_invalidate.render ||
      !fixture.view.invalidation_after_entity_invalidate.layout ||
      !fixture.view.invalidation_after_entity_invalidate.paint ||
      fixture.view.entity_handle_observer_count_after_invalidate != 1 ||
      fixture.view.entity_handle_observer_value_sum_after_invalidate != 22) {
    return 293;
  }
  if (!fixture.view.context_invalidate_entity ||
      !fixture.view.invalidation_after_context_entity_invalidate.render ||
      !fixture.view.invalidation_after_context_entity_invalidate.layout ||
      !fixture.view.invalidation_after_context_entity_invalidate.paint ||
      fixture.view.entity_handle_observer_count_after_context_invalidate != 2 ||
      fixture.view.entity_handle_observer_value_sum_after_context_invalidate !=
          44) {
    return 294;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1) {
    return 295;
  }

  return 0;
}

int test_entity_can_observe_another_entity() {
  RuntimeFixture fixture;
  entity_handle_fixture = &fixture;
  fixture.app.on_run = &dispatch_entity_handle_sequence;
  fixture.view.exercise_entity_to_entity_observation = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  entity_handle_fixture = nullptr;

  if (result != 0) {
    return 330;
  }
  if (fixture.view.observing_entity.empty() ||
      fixture.view.observed_entity.empty()) {
    return 331;
  }
  if (!fixture.view.entity_to_entity_observed ||
      !fixture.view.entity_to_entity_subscription_connected ||
      !fixture.view.entity_to_entity_context_matched) {
    return 332;
  }
  if (!fixture.view.entity_to_entity_observed_update) {
    return 333;
  }
  if (fixture.view.entity_to_entity_callback_count_after_update != 1) {
    return 335;
  }
  if (fixture.view.entity_to_entity_subscription_count_after_update != 1) {
    return 336;
  }
  if (fixture.view.entity_to_entity_observer_value_after_update != 19) {
    return 337;
  }
  if (!fixture.view.entity_to_entity_release ||
      !fixture.view.entity_to_entity_update_after_release ||
      fixture.view.entity_to_entity_callback_count != 2 ||
      fixture.view.entity_to_entity_subscription_count != 1 ||
      fixture.view.entity_to_entity_observer_value_after_release != 29) {
    return 334;
  }
  return 0;
}

int test_entity_handles_do_not_cross_runtime_boundaries() {
  RuntimeFixture first_fixture;
  CrossRuntimeEntityBoundaryView first_view;
  first_view.create_exported_entity = true;
  cross_runtime_entity_fixture = &first_fixture;
  first_fixture.app.on_run = &dispatch_cross_runtime_entity_sequence;

  cgpui::WindowRuntime first_runtime(
      first_fixture.app,
      first_view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&first_fixture.renderer};
      });

  const int first_result =
      first_runtime.run(cgpui::WindowDescriptor{},
                        cgpui::WindowRuntimeOptions{
                            .request_initial_redraw = false});
  cross_runtime_entity_fixture = nullptr;

  if (first_result != 0) {
    return 296;
  }
  if (first_view.exported_entity.empty() ||
      first_view.exported_entity_read_value != 41) {
    return 297;
  }

  RuntimeFixture second_fixture;
  CrossRuntimeEntityBoundaryView second_view;
  second_view.exercise_imported_entity = true;
  second_view.imported_entity = first_view.exported_entity;
  cross_runtime_entity_fixture = &second_fixture;
  second_fixture.app.on_run = &dispatch_cross_runtime_entity_sequence;

  cgpui::WindowRuntime second_runtime(
      second_fixture.app,
      second_view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&second_fixture.renderer};
      });

  const int second_result =
      second_runtime.run(cgpui::WindowDescriptor{},
                         cgpui::WindowRuntimeOptions{
                             .request_initial_redraw = false});
  cross_runtime_entity_fixture = nullptr;

  if (second_result != 0) {
    return 298;
  }
  if (!second_view.same_numeric_entity_id) {
    return 299;
  }
  if (second_view.imported_read || second_view.imported_update ||
      second_view.imported_invalidate || second_view.imported_remove ||
      second_view.imported_weak_upgrade) {
    return 300;
  }
  if (!second_view.local_entity_still_present ||
      second_view.local_entity_value != 99) {
    return 301;
  }

  return 0;
}

RuntimeFixture* global_state_fixture = nullptr;

void dispatch_global_state_sequence() {
  auto& callback = global_state_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 71,
      .action = cgpui::KeyAction::pressed});
}

int test_view_context_global_state_helpers() {
  RuntimeFixture fixture;
  global_state_fixture = &fixture;
  fixture.app.on_run = &dispatch_global_state_sequence;
  fixture.view.exercise_global_state_helpers = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  global_state_fixture = nullptr;

  if (result != 0) {
    return 284;
  }
  if (!fixture.view.global_missing_before_set ||
      fixture.view.global_missing_update) {
    return 285;
  }
  if (fixture.view.global_first_read_value != 12 ||
      !fixture.view.global_update ||
      fixture.view.global_updated_value != 42) {
    return 286;
  }
  if (fixture.view.global_replaced_value != 77) {
    return 287;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1) {
    return 288;
  }
  return 0;
}

RuntimeFixture* view_identity_fixture = nullptr;

RuntimeFixture* action_dispatch_fixture = nullptr;

void dispatch_action_dispatch_sequence() {
  auto& callback = action_dispatch_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 83,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
}

int test_runtime_dispatches_named_actions() {
  RuntimeFixture fixture;
  action_dispatch_fixture = &fixture;
  fixture.app.on_run = &dispatch_action_dispatch_sequence;
  fixture.view.exercise_action_dispatch = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  action_dispatch_fixture = nullptr;

  if (result != 0) {
    return 150;
  }
  if (fixture.view.keyboard_key_count != 1 ||
      fixture.view.dispatched_action_count != 1) {
    return 151;
  }
  if (fixture.view.action_saw_context_view_id != fixture.view.first_view_id) {
    return 152;
  }
  if (fixture.view.first_action_result.name != "app.save" ||
      !fixture.view.first_action_result.handled ||
      !fixture.view.first_action_result.result.consumed ||
      fixture.view.first_action_result.result.cancelled) {
    return 153;
  }
  if (fixture.view.second_action_result.name != "app.missing" ||
      fixture.view.second_action_result.handled ||
      fixture.view.second_action_result.result.consumed ||
      fixture.view.second_action_result.result.cancelled) {
    return 154;
  }
  if (!fixture.view.last_action_result_from_context.has_value() ||
      fixture.view.last_action_result_from_context->name != "app.missing" ||
      fixture.view.last_action_result_from_context->handled) {
    return 155;
  }

  return 0;
}

int test_view_context_registers_and_dispatches_actions() {
  RuntimeFixture fixture;
  action_dispatch_fixture = &fixture;
  fixture.app.on_run = &dispatch_action_dispatch_sequence;
  fixture.view.exercise_view_context_action_helper = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  action_dispatch_fixture = nullptr;

  if (result != 0) {
    return 232;
  }
  if (fixture.view.keyboard_key_count != 1 ||
      fixture.view.view_context_action_count != 1) {
    return 233;
  }
  if (fixture.view.view_context_action_saw_context_view_id !=
      fixture.view.first_view_id) {
    return 234;
  }
  if (fixture.view.view_context_first_action_result.name != "view.open" ||
      !fixture.view.view_context_first_action_result.handled ||
      !fixture.view.view_context_first_action_result.result.consumed ||
      fixture.view.view_context_first_action_result.result.cancelled) {
    return 235;
  }
  if (fixture.view.view_context_second_action_result.name != "view.missing" ||
      fixture.view.view_context_second_action_result.handled ||
      fixture.view.view_context_second_action_result.result.consumed ||
      fixture.view.view_context_second_action_result.result.cancelled) {
    return 236;
  }
  if (!fixture.view.view_context_last_action_result.has_value() ||
      fixture.view.view_context_last_action_result->name != "view.missing" ||
      fixture.view.view_context_last_action_result->handled) {
    return 237;
  }

  return 0;
}

RuntimeFixture* scoped_action_fixture = nullptr;

void dispatch_scoped_action_sequence() {
  auto& callback = scoped_action_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed});
}

class ScopedActionView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    context.runtime.request_keyboard_focus(focused_element_id);
    context.runtime.register_action(
        "legacy.app",
        [this](const cgpui::WindowRuntimeContext&) {
          app_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_action(
        "scoped.all",
        [this](const cgpui::WindowRuntimeContext&) {
          app_shadow_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_window_action(
        "scoped.all",
        [this](const cgpui::WindowRuntimeContext&) {
          window_shadow_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_view_action(
        context.view_id,
        "scoped.all",
        [this](const cgpui::WindowRuntimeContext&) {
          view_shadow_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_focused_element_action(
        focused_element_id,
        "scoped.all",
        [this](const cgpui::WindowRuntimeContext&) {
          focused_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_action(
        "scoped.view",
        [this](const cgpui::WindowRuntimeContext&) {
          app_shadow_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_window_action(
        "scoped.view",
        [this](const cgpui::WindowRuntimeContext&) {
          window_shadow_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_view_action(
        context.view_id,
        "scoped.view",
        [this](const cgpui::WindowRuntimeContext&) {
          view_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_action(
        "scoped.window",
        [this](const cgpui::WindowRuntimeContext&) {
          app_shadow_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_window_action(
        "scoped.window",
        [this](const cgpui::WindowRuntimeContext&) {
          window_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });

    focused_result = context.runtime.dispatch_action("scoped.all");
    view_result = context.runtime.dispatch_action("scoped.view");
    window_result = context.runtime.dispatch_action("scoped.window");
    app_result = context.runtime.dispatch_action("legacy.app");
    missing_result = context.runtime.dispatch_action("scoped.missing");
    last_result = context.runtime.last_action_dispatch();
    return cgpui::EventResult::consumed_event();
  }

  cgpui::ElementId focused_element_id{42};
  int app_action_count = 0;
  int app_shadow_count = 0;
  int window_action_count = 0;
  int window_shadow_count = 0;
  int view_action_count = 0;
  int view_shadow_count = 0;
  int focused_action_count = 0;
  cgpui::ActionDispatchResult focused_result{};
  cgpui::ActionDispatchResult view_result{};
  cgpui::ActionDispatchResult window_result{};
  cgpui::ActionDispatchResult app_result{};
  cgpui::ActionDispatchResult missing_result{};
  std::optional<cgpui::ActionDispatchResult> last_result;
};

int test_runtime_dispatches_scoped_actions_by_lookup_order() {
  RuntimeFixture fixture;
  ScopedActionView view;
  scoped_action_fixture = &fixture;
  fixture.app.on_run = &dispatch_scoped_action_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  scoped_action_fixture = nullptr;

  if (result != 0) {
    return 349;
  }
  if (view.focused_action_count != 1 || view.view_action_count != 1 ||
      view.window_action_count != 1 || view.app_action_count != 1) {
    return 350;
  }
  if (view.app_shadow_count != 0 || view.window_shadow_count != 0 ||
      view.view_shadow_count != 0) {
    return 351;
  }
  if (!view.focused_result.scope.has_value() ||
      *view.focused_result.scope != cgpui::ActionScope::focused_element ||
      !view.focused_result.element_id.has_value() ||
      *view.focused_result.element_id != view.focused_element_id) {
    return 352;
  }
  if (!view.view_result.scope.has_value() ||
      *view.view_result.scope != cgpui::ActionScope::view ||
      !view.view_result.view_id.has_value() ||
      *view.view_result.view_id != cgpui::ViewId{1}) {
    return 353;
  }
  if (!view.window_result.scope.has_value() ||
      *view.window_result.scope != cgpui::ActionScope::window ||
      view.window_result.view_id.has_value() ||
      view.window_result.element_id.has_value()) {
    return 354;
  }
  if (!view.app_result.scope.has_value() ||
      *view.app_result.scope != cgpui::ActionScope::app ||
      !view.app_result.handled ||
      !view.app_result.result.consumed) {
    return 355;
  }
  if (view.missing_result.handled || view.missing_result.scope.has_value()) {
    return 356;
  }
  if (!view.last_result.has_value() ||
      view.last_result->name != "scoped.missing" ||
      view.last_result->scope.has_value()) {
    return 357;
  }

  return 0;
}

class ViewContextScopedActionView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    context.focus(focused_element_id);
    context.register_action(
        "context.legacy",
        [this](const cgpui::ViewContext&) {
          app_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_app_action(
        "context.app",
        [this](const cgpui::ViewContext&) {
          explicit_app_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_window_action(
        "context.window",
        [this](const cgpui::ViewContext&) {
          window_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_view_action(
        "context.view",
        [this](const cgpui::ViewContext&) {
          view_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_focused_element_action(
        focused_element_id,
        "context.focused",
        [this](const cgpui::ViewContext&) {
          focused_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });

    legacy_result = context.dispatch_action("context.legacy");
    app_result = context.dispatch_action("context.app");
    window_result = context.dispatch_action("context.window");
    view_result = context.dispatch_action("context.view");
    focused_result = context.dispatch_action("context.focused");
    return cgpui::EventResult::consumed_event();
  }

  cgpui::ElementId focused_element_id{51};
  int app_action_count = 0;
  int explicit_app_action_count = 0;
  int window_action_count = 0;
  int view_action_count = 0;
  int focused_action_count = 0;
  cgpui::ActionDispatchResult legacy_result{};
  cgpui::ActionDispatchResult app_result{};
  cgpui::ActionDispatchResult window_result{};
  cgpui::ActionDispatchResult view_result{};
  cgpui::ActionDispatchResult focused_result{};
};

int test_view_context_registers_explicit_scoped_actions() {
  RuntimeFixture fixture;
  ViewContextScopedActionView view;
  scoped_action_fixture = &fixture;
  fixture.app.on_run = &dispatch_scoped_action_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  scoped_action_fixture = nullptr;

  if (result != 0) {
    return 358;
  }
  if (view.app_action_count != 1 || view.explicit_app_action_count != 1 ||
      view.window_action_count != 1 || view.view_action_count != 1 ||
      view.focused_action_count != 1) {
    return 359;
  }
  if (!view.legacy_result.scope.has_value() ||
      *view.legacy_result.scope != cgpui::ActionScope::app ||
      !view.app_result.scope.has_value() ||
      *view.app_result.scope != cgpui::ActionScope::app) {
    return 360;
  }
  if (!view.window_result.scope.has_value() ||
      *view.window_result.scope != cgpui::ActionScope::window ||
      !view.view_result.scope.has_value() ||
      *view.view_result.scope != cgpui::ActionScope::view ||
      !view.focused_result.scope.has_value() ||
      *view.focused_result.scope != cgpui::ActionScope::focused_element) {
    return 361;
  }
  if (!view.focused_result.element_id.has_value() ||
      *view.focused_result.element_id != view.focused_element_id) {
    return 362;
  }

  return 0;
}

class CommandPaletteRegistryView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    context.focus(focused_element_id);
    context.register_app_action(
        "palette.app",
        [this](const cgpui::ViewContext&) {
          app_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_window_action(
        "palette.window",
        [this](const cgpui::ViewContext&) {
          window_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_view_action(
        "palette.view",
        [this](const cgpui::ViewContext&) {
          view_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_focused_element_action(
        focused_element_id,
        "palette.focused",
        [this](const cgpui::ViewContext&) {
          focused_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_app_action(
        "palette.disabled",
        [this](const cgpui::ViewContext&) {
          disabled_action_count += 1;
          return cgpui::EventResult::consumed_event();
        });

    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "palette.app",
        .title = "Open Settings",
        .group = "File",
        .scope = cgpui::ActionScope::app,
    });
    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "palette.window",
        .title = "Close Window",
        .group = "File",
        .scope = cgpui::ActionScope::window,
    });
    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "palette.view",
        .title = "Focus Editor",
        .group = "Navigate",
        .scope = cgpui::ActionScope::view,
        .view_id = context.view_id,
    });
    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "palette.focused",
        .title = "Rename Symbol",
        .group = "Edit",
        .scope = cgpui::ActionScope::focused_element,
        .element_id = focused_element_id,
    });
    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "palette.disabled",
        .title = "Disabled Command",
        .group = "File",
        .scope = cgpui::ActionScope::app,
        .enabled = false,
    });
    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "",
        .title = "Ignored Command",
        .group = "File",
    });

    entries = std::vector<cgpui::CommandPaletteEntry>(
        context.command_palette_entries().begin(),
        context.command_palette_entries().end());
    file_entries = context.command_palette_entries_for_group("File");
    app_result = context.dispatch_command_palette_action("palette.app");
    window_result = context.dispatch_command_palette_entry(entries[1]);
    view_result = context.dispatch_command_palette_entry(entries[2]);
    focused_result = context.dispatch_command_palette_entry(entries[3]);
    disabled_result = context.dispatch_command_palette_entry(entries[4]);
    missing_result = context.dispatch_command_palette_action("palette.missing");
    last_result = context.last_action_dispatch();

    return cgpui::EventResult::consumed_event();
  }

  cgpui::ElementId focused_element_id{61};
  int app_action_count = 0;
  int window_action_count = 0;
  int view_action_count = 0;
  int focused_action_count = 0;
  int disabled_action_count = 0;
  std::vector<cgpui::CommandPaletteEntry> entries;
  std::vector<cgpui::CommandPaletteEntry> file_entries;
  cgpui::ActionDispatchResult app_result{};
  cgpui::ActionDispatchResult window_result{};
  cgpui::ActionDispatchResult view_result{};
  cgpui::ActionDispatchResult focused_result{};
  cgpui::ActionDispatchResult disabled_result{};
  cgpui::ActionDispatchResult missing_result{};
  std::optional<cgpui::ActionDispatchResult> last_result;
};

int test_runtime_registers_command_palette_entries() {
  RuntimeFixture fixture;
  CommandPaletteRegistryView view;
  scoped_action_fixture = &fixture;
  fixture.app.on_run = &dispatch_scoped_action_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  scoped_action_fixture = nullptr;

  if (result != 0) {
    return 363;
  }
  if (view.entries.size() != 5 || runtime.command_palette_entries().size() != 5) {
    return 364;
  }
  if (view.entries[0].action_name != "palette.app" ||
      view.entries[0].title != "Open Settings" ||
      view.entries[0].group != "File" ||
      view.entries[0].scope != cgpui::ActionScope::app) {
    return 365;
  }
  if (view.file_entries.size() != 3 ||
      view.file_entries[0].action_name != "palette.app" ||
      view.file_entries[2].action_name != "palette.disabled") {
    return 366;
  }
  if (view.app_action_count != 1 || view.window_action_count != 1 ||
      view.view_action_count != 1 || view.focused_action_count != 1 ||
      view.disabled_action_count != 0) {
    return 367;
  }
  if (!view.app_result.handled || !view.app_result.scope.has_value() ||
      *view.app_result.scope != cgpui::ActionScope::app ||
      !view.window_result.scope.has_value() ||
      *view.window_result.scope != cgpui::ActionScope::window ||
      !view.view_result.scope.has_value() ||
      *view.view_result.scope != cgpui::ActionScope::view ||
      !view.focused_result.scope.has_value() ||
      *view.focused_result.scope != cgpui::ActionScope::focused_element) {
    return 368;
  }
  if (!view.view_result.view_id.has_value() ||
      *view.view_result.view_id != cgpui::ViewId{1} ||
      !view.focused_result.element_id.has_value() ||
      *view.focused_result.element_id != view.focused_element_id) {
    return 369;
  }
  if (view.disabled_result.name != "palette.disabled" ||
      view.disabled_result.handled || view.disabled_result.result.consumed ||
      view.missing_result.handled || view.missing_result.scope.has_value()) {
    return 370;
  }
  if (!view.last_result.has_value() ||
      view.last_result->name != "palette.missing" ||
      view.last_result->handled) {
    return 371;
  }

  return 0;
}

RuntimeFixture* deferred_callback_fixture = nullptr;
class TimerApiView;
TimerApiView* timer_api_view = nullptr;
cgpui::WindowRuntime* timer_api_runtime = nullptr;
class AnimationApiView;
AnimationApiView* animation_api_view = nullptr;
cgpui::WindowRuntime* animation_api_runtime = nullptr;
class AsyncTaskApiView;
AsyncTaskApiView* async_task_api_view = nullptr;
cgpui::WindowRuntime* async_task_api_runtime = nullptr;
class ThreadedAsyncExecutorView;
ThreadedAsyncExecutorView* threaded_async_executor_view = nullptr;
cgpui::WindowRuntime* threaded_async_executor_runtime = nullptr;

} // namespace

int main() {
  if (const int result = test_context_can_access_runtime_entities(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_tracks_view_model_subscriptions(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_model_helpers_create_read_update_and_remove(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_observes_model_changes(); result != 0) {
    return result;
  }
  if (const int result = test_subscription_token_disconnects_observers_on_drop_and_removal(); result != 0) {
    return result;
  }
  if (const int result = test_weak_entity_and_view_handles_upgrade_softly(); result != 0) {
    return result;
  }
  if (const int result = test_entity_handle_read_update_and_downgrade(); result != 0) {
    return result;
  }
  if (const int result = test_entity_handle_and_context_invalidate_entities(); result != 0) {
    return result;
  }
  if (const int result = test_entity_can_observe_another_entity(); result != 0) {
    return result;
  }
  if (const int result = test_entity_handles_do_not_cross_runtime_boundaries(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_global_state_helpers(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_dispatches_named_actions(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_registers_and_dispatches_actions(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_dispatches_scoped_actions_by_lookup_order(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_registers_explicit_scoped_actions(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_registers_command_palette_entries(); result != 0) {
    return result;
  }
  return 0;
}
