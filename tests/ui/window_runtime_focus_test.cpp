#include "window_runtime_test_support.hpp"

namespace {
RuntimeFixture* click_focus_fixture = nullptr;
RuntimeFixture* focus_traversal_fixture = nullptr;
RuntimeFixture* scroll_routing_fixture = nullptr;

void dispatch_click_focus_sequence() {
  auto& callback = click_focus_fixture->window.callback;
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = true,
      .position = {5.0F, 5.0F}});
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = false,
      .position = {5.0F, 5.0F}});
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed});
}

void dispatch_focus_traversal_sequence() {
  auto& callback = focus_traversal_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 9,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 9,
      .action = cgpui::KeyAction::pressed});
}

void dispatch_reverse_focus_traversal_sequence() {
  auto& callback = focus_traversal_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 9,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.shift = true}});
}

void dispatch_scroll_routing_sequence() {
  auto& callback = scroll_routing_fixture->window.callback;
  callback(cgpui::PointerScrolled{
      .delta = {0.0F, 26.0F},
      .position = {5.0F, 5.0F}});
}

int test_runtime_clicks_request_focus_for_focusable_elements() {
  RuntimeFixture fixture;
  click_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_click_focus_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  auto root = std::make_unique<RuntimeFocusableElement>(
      cgpui::Size{.width = 40.0F, .height = 20.0F});
  RuntimeFocusableElement* root_ptr = root.get();
  const cgpui::ElementId root_id = tree->set_root(std::move(root));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  std::optional<cgpui::ElementId> focus_owner_after_click;
  std::optional<cgpui::ElementId> keyboard_route_after_focus;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::pointer_button) {
          focus_owner_after_click = context.input.keyboard_focus_element_owner;
        } else if (record.event_kind == cgpui::EventKind::keyboard_key) {
          keyboard_route_after_focus = record.route.target_element_id;
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = true});
  click_focus_fixture = nullptr;

  if (result != 0) {
    return 218;
  }
  if (root_id.value == 0 || root_ptr->focus_count != 1 ||
      root_ptr->last_focused_element_id != root_id) {
    return 219;
  }
  if (!focus_owner_after_click.has_value() ||
      *focus_owner_after_click != root_id) {
    return 220;
  }
  if (!keyboard_route_after_focus.has_value() ||
      *keyboard_route_after_focus != root_id) {
    return 221;
  }
  return 0;
}

int test_runtime_clicks_button_widget_focus_and_dispatch_action() {
  RuntimeFixture fixture;
  click_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_click_focus_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  cgpui::AnyElement button =
      cgpui::button("dialog.accept")
          .style(cgpui::Style{}.with_preferred_size(
              cgpui::Size{.width = 40.0F, .height = 20.0F}))
          .build();
  const cgpui::ElementId button_id = tree->set_root(std::move(button));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  int action_count = 0;
  runtime.register_view_action(
      cgpui::ViewId{1},
      "dialog.accept",
      [&](const cgpui::WindowRuntimeContext& context) {
        action_count += 1;
        return context.input.keyboard_focus_element_owner == button_id
                   ? cgpui::EventResult::consumed_event()
                   : cgpui::EventResult::cancelled_event();
      });

  std::optional<cgpui::ElementId> focus_owner_after_click;
  std::optional<cgpui::ActionDispatchResult> action_after_click;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::pointer_button) {
          focus_owner_after_click = context.input.keyboard_focus_element_owner;
          action_after_click = context.last_action_dispatch();
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = true});
  click_focus_fixture = nullptr;

  if (result != 0) {
    return 389;
  }
  if (button_id.value == 0 || !focus_owner_after_click.has_value() ||
      *focus_owner_after_click != button_id) {
    return 390;
  }
  if (action_count != 1 || !action_after_click.has_value() ||
      action_after_click->name != "dialog.accept" ||
      !action_after_click->handled || !action_after_click->scope.has_value() ||
      *action_after_click->scope != cgpui::ActionScope::view ||
      !action_after_click->view_id.has_value() ||
      *action_after_click->view_id != cgpui::ViewId{1} ||
      !action_after_click->result.consumed ||
      action_after_click->result.cancelled) {
    return 391;
  }
  if (fixture.view.keyboard_key_count != 1 ||
      fixture.view.last_route_element_id != button_id) {
    return 392;
  }
  return 0;
}

int test_runtime_tabs_focus_forward_over_enabled_focusable_elements() {
  RuntimeFixture fixture;
  focus_traversal_fixture = &fixture;
  fixture.app.on_run = &dispatch_focus_traversal_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 100.0F, .height = 100.0F}));
  const cgpui::ElementId first_id = tree->append_child(
      root_id,
      std::make_unique<RuntimeFocusableElement>(
          cgpui::Size{.width = 20.0F, .height = 20.0F}));
  auto disabled = std::make_unique<RuntimeFocusableElement>(
      cgpui::Size{.width = 20.0F, .height = 20.0F});
  disabled->set_enabled(false);
  const cgpui::ElementId disabled_id =
      tree->append_child(root_id, std::move(disabled));
  const cgpui::ElementId second_id = tree->append_child(
      root_id,
      std::make_unique<RuntimeFocusableElement>(
          cgpui::Size{.width = 20.0F, .height = 20.0F}));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  std::optional<cgpui::ElementId> focus_after_first_tab;
  std::optional<cgpui::ElementId> focus_after_second_tab;
  int tab_count = 0;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind != cgpui::EventKind::keyboard_key) {
          return;
        }
        tab_count += 1;
        if (tab_count == 1) {
          focus_after_first_tab = context.input.keyboard_focus_element_owner;
        } else if (tab_count == 2) {
          focus_after_second_tab = context.input.keyboard_focus_element_owner;
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  focus_traversal_fixture = nullptr;

  if (result != 0) {
    return 360;
  }
  if (first_id.value == 0 || disabled_id.value == 0 || second_id.value == 0) {
    return 361;
  }
  if (!focus_after_first_tab.has_value() ||
      *focus_after_first_tab != first_id) {
    return 362;
  }
  if (!focus_after_second_tab.has_value() ||
      *focus_after_second_tab != second_id ||
      *focus_after_second_tab == disabled_id) {
    return 363;
  }
  return 0;
}

int test_runtime_shift_tab_focuses_previous_enabled_focusable_element() {
  RuntimeFixture fixture;
  focus_traversal_fixture = &fixture;
  fixture.app.on_run = &dispatch_reverse_focus_traversal_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 100.0F, .height = 100.0F}));
  const cgpui::ElementId first_id = tree->append_child(
      root_id,
      std::make_unique<RuntimeFocusableElement>(
          cgpui::Size{.width = 20.0F, .height = 20.0F}));
  const cgpui::ElementId second_id = tree->append_child(
      root_id,
      std::make_unique<RuntimeFocusableElement>(
          cgpui::Size{.width = 20.0F, .height = 20.0F}));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  runtime.request_keyboard_focus(first_id);

  std::optional<cgpui::ElementId> focus_after_shift_tab;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::keyboard_key) {
          focus_after_shift_tab = context.input.keyboard_focus_element_owner;
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  focus_traversal_fixture = nullptr;

  if (result != 0) {
    return 364;
  }
  if (first_id.value == 0 || second_id.value == 0) {
    return 365;
  }
  if (!focus_after_shift_tab.has_value() ||
      *focus_after_shift_tab != second_id) {
    return 366;
  }
  return 0;
}

int test_runtime_tabs_follow_explicit_tab_index_before_tree_order() {
  RuntimeFixture fixture;
  focus_traversal_fixture = &fixture;
  fixture.app.on_run = &dispatch_focus_traversal_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 100.0F, .height = 100.0F}));
  const cgpui::ElementId default_id = tree->append_child(
      root_id,
      std::make_unique<RuntimeFocusableElement>(
          cgpui::Size{.width = 20.0F, .height = 20.0F}));
  auto first_tab = std::make_unique<RuntimeFocusableElement>(
      cgpui::Size{.width = 20.0F, .height = 20.0F});
  first_tab->set_focus_metadata(cgpui::FocusMetadata{
      .tab_index = 1,
      .focus_ring = cgpui::FocusRingVisibility::visible,
  });
  const cgpui::ElementId first_tab_id =
      tree->append_child(root_id, std::move(first_tab));
  auto second_tab = std::make_unique<RuntimeFocusableElement>(
      cgpui::Size{.width = 20.0F, .height = 20.0F});
  second_tab->set_focus_metadata(cgpui::FocusMetadata{
      .tab_index = 2,
  });
  const cgpui::ElementId second_tab_id =
      tree->append_child(root_id, std::move(second_tab));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  std::optional<cgpui::ElementId> focus_after_first_tab;
  std::optional<cgpui::ElementId> focus_after_second_tab;
  int tab_count = 0;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind != cgpui::EventKind::keyboard_key) {
          return;
        }
        tab_count += 1;
        if (tab_count == 1) {
          focus_after_first_tab = context.input.keyboard_focus_element_owner;
        } else if (tab_count == 2) {
          focus_after_second_tab = context.input.keyboard_focus_element_owner;
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  focus_traversal_fixture = nullptr;

  if (result != 0) {
    return 413;
  }
  if (default_id.value == 0 || first_tab_id.value == 0 ||
      second_tab_id.value == 0) {
    return 414;
  }
  if (!focus_after_first_tab.has_value() ||
      *focus_after_first_tab != first_tab_id) {
    return 415;
  }
  if (!focus_after_second_tab.has_value() ||
      *focus_after_second_tab != second_tab_id ||
      *focus_after_second_tab == default_id) {
    return 416;
  }
  return 0;
}

int test_runtime_tabs_skip_negative_tab_index_metadata() {
  RuntimeFixture fixture;
  focus_traversal_fixture = &fixture;
  fixture.app.on_run = &dispatch_focus_traversal_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 100.0F, .height = 100.0F}));
  auto skipped = std::make_unique<RuntimeFocusableElement>(
      cgpui::Size{.width = 20.0F, .height = 20.0F});
  skipped->set_focus_metadata(cgpui::FocusMetadata{.tab_index = -1});
  const cgpui::ElementId skipped_id =
      tree->append_child(root_id, std::move(skipped));
  const cgpui::ElementId target_id = tree->append_child(
      root_id,
      std::make_unique<RuntimeFocusableElement>(
          cgpui::Size{.width = 20.0F, .height = 20.0F}));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  std::optional<cgpui::ElementId> focus_after_first_tab;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::keyboard_key &&
            !focus_after_first_tab.has_value()) {
          focus_after_first_tab = context.input.keyboard_focus_element_owner;
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  focus_traversal_fixture = nullptr;

  if (result != 0) {
    return 417;
  }
  if (skipped_id.value == 0 || target_id.value == 0) {
    return 418;
  }
  if (!focus_after_first_tab.has_value() ||
      *focus_after_first_tab != target_id ||
      *focus_after_first_tab == skipped_id) {
    return 419;
  }
  return 0;
}

int test_runtime_does_not_focus_disabled_focusable_elements() {
  RuntimeFixture fixture;
  click_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_click_focus_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  auto root = std::make_unique<RuntimeFocusableElement>(
      cgpui::Size{.width = 40.0F, .height = 20.0F});
  root->set_enabled(false);
  RuntimeFocusableElement* root_ptr = root.get();
  const cgpui::ElementId root_id = tree->set_root(std::move(root));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  std::optional<cgpui::ElementId> route_after_click;
  std::optional<cgpui::ElementId> focus_owner_after_click;
  std::optional<cgpui::ElementId> keyboard_route_after_click;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::pointer_button) {
          route_after_click = record.route.target_element_id;
          focus_owner_after_click = context.input.keyboard_focus_element_owner;
        } else if (record.event_kind == cgpui::EventKind::keyboard_key) {
          keyboard_route_after_click = record.route.target_element_id;
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = true});
  click_focus_fixture = nullptr;

  if (result != 0) {
    return 280;
  }
  if (root_id.value == 0 || !route_after_click.has_value() ||
      *route_after_click != root_id) {
    return 281;
  }
  if (root_ptr->focus_count != 0 ||
      root_ptr->last_focused_element_id.value != 0) {
    return 282;
  }
  if (focus_owner_after_click.has_value()) {
    return 283;
  }
  if (keyboard_route_after_click.has_value()) {
    return 284;
  }
  return 0;
}

int test_runtime_scrolls_bound_scroll_state_at_hit_element() {
  RuntimeFixture fixture;
  scroll_routing_fixture = &fixture;
  fixture.app.on_run = &dispatch_scroll_routing_sequence;

  cgpui::ScrollState scroll_state;
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId scroll_id = tree->set_root(cgpui::scroll(
      scroll_state,
      std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 800.0F, .height = 900.0F})));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  std::optional<cgpui::ElementId> routed_element_id;
  cgpui::EventResult scroll_result{};
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::pointer_scrolled) {
          routed_element_id = record.route.target_element_id;
          scroll_result = record.result;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{
      .size = {.width = 40.0F, .height = 40.0F}});
  scroll_routing_fixture = nullptr;

  if (result != 0) {
    return 367;
  }
  if (scroll_id.value == 0 || !routed_element_id.has_value() ||
      *routed_element_id != scroll_id) {
    return 368;
  }
  if (scroll_state.offset().x != 0.0F || scroll_state.offset().y != 26.0F) {
    return 369;
  }
  if (!scroll_result.consumed || scroll_result.cancelled) {
    return 370;
  }
  if (fixture.view.pointer_scroll_count != 0) {
    return 371;
  }
  return 0;
}

int test_runtime_scrolls_nearest_scroll_ancestor_for_child_hit() {
  RuntimeFixture fixture;
  scroll_routing_fixture = &fixture;
  fixture.app.on_run = &dispatch_scroll_routing_sequence;

  cgpui::ScrollState scroll_state;
  auto tree = std::make_unique<cgpui::ElementTree>();
  auto child = cgpui::div().size(800.0F, 900.0F).build();
  child->assign_id(cgpui::ElementId{999});
  const cgpui::ElementId scroll_id =
      tree->set_root(cgpui::scroll(scroll_state, std::move(child)));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  std::optional<cgpui::ElementId> routed_element_id;
  std::vector<cgpui::ElementId> route_ancestry;
  cgpui::EventResult scroll_result{};
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::pointer_scrolled) {
          routed_element_id = record.route.target_element_id;
          route_ancestry = record.route.element_ancestry;
          scroll_result = record.result;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{
      .size = {.width = 40.0F, .height = 40.0F}});
  scroll_routing_fixture = nullptr;

  if (result != 0) {
    return 372;
  }
  if (scroll_id.value == 0 || !routed_element_id.has_value()) {
    return 373;
  }
  if (*routed_element_id != scroll_id) {
    return 374;
  }
  if (scroll_state.offset().x != 0.0F || scroll_state.offset().y != 26.0F) {
    return 375;
  }
  if (route_ancestry.size() != 1 || route_ancestry[0] != scroll_id) {
    return 376;
  }
  return scroll_result.consumed && !scroll_result.cancelled &&
                 fixture.view.pointer_scroll_count == 0
             ? 0
             : 377;
}

RuntimeFixture* keyboard_focus_fixture = nullptr;

void dispatch_keyboard_focus_sequence() {
  auto& callback = keyboard_focus_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 66,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "b"});
  callback(cgpui::KeyboardKey{
      .key_code = 67,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 68,
      .action = cgpui::KeyAction::pressed});
}

int test_view_can_request_and_release_keyboard_focus() {
  RuntimeFixture fixture;
  keyboard_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_keyboard_focus_sequence;
  fixture.view.request_keyboard_focus_on_first_key = true;
  fixture.view.release_keyboard_focus_on_third_key = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  keyboard_focus_fixture = nullptr;

  if (result != 0) {
    return 70;
  }
  if (fixture.view.keyboard_key_count != 4 ||
      fixture.view.text_input_count != 1) {
    return 71;
  }
  if (!fixture.view.second_key_saw_keyboard_focus ||
      !fixture.view.text_input_saw_keyboard_focus) {
    return 72;
  }
  if (fixture.view.fourth_key_saw_keyboard_focus ||
      fixture.view.last_keyboard_focused) {
    return 73;
  }

  return 0;
}

int test_keyboard_focus_tracks_owner_view_id() {
  RuntimeFixture fixture;
  keyboard_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_keyboard_focus_sequence;
  fixture.view.request_keyboard_focus_owner_on_first_key = true;
  fixture.view.release_keyboard_focus_with_wrong_owner_on_second_key = true;
  fixture.view.release_keyboard_focus_owner_on_third_key = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  keyboard_focus_fixture = nullptr;

  if (result != 0) {
    return 80;
  }
  if (!fixture.view.saw_first_view_id ||
      fixture.view.first_view_id.value == 0 ||
      !fixture.view.view_id_stayed_stable) {
    return 81;
  }
  if (!fixture.view.second_key_saw_keyboard_focus ||
      !fixture.view.second_key_saw_keyboard_focus_owner) {
    return 82;
  }
  if (!fixture.view.text_input_saw_keyboard_focus ||
      !fixture.view.text_input_saw_keyboard_focus_owner) {
    return 83;
  }
  if (!fixture.view.third_key_saw_keyboard_focus ||
      !fixture.view.third_key_saw_keyboard_focus_owner) {
    return 84;
  }
  if (fixture.view.fourth_key_saw_keyboard_focus ||
      fixture.view.fourth_key_saw_keyboard_focus_owner ||
      fixture.view.last_keyboard_focus_owner_present) {
    return 85;
  }

  return 0;
}

int test_keyboard_focus_routes_to_owner_element() {
  RuntimeFixture fixture;
  keyboard_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_keyboard_focus_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;
  fixture.view.release_keyboard_focus_element_with_wrong_owner_on_second_key =
      true;
  fixture.view.release_keyboard_focus_element_owner_on_third_key = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  int callback_count = 0;
  cgpui::EventDispatchRecord first_key_record{};
  cgpui::EventDispatchRecord second_key_record{};
  cgpui::EventDispatchRecord text_record{};
  cgpui::EventDispatchRecord third_key_record{};
  cgpui::EventDispatchRecord fourth_key_record{};
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        if (!context.event_route.has_value() ||
            context.event_route->target_view_id != record.route.target_view_id ||
            context.event_route->target_element_id !=
                record.route.target_element_id ||
            context.event_route->event_kind != record.route.event_kind) {
          callback_count = -100;
          return;
        }
        if (callback_count == 1) {
          first_key_record = record;
        } else if (callback_count == 2) {
          second_key_record = record;
        } else if (callback_count == 3) {
          text_record = record;
        } else if (callback_count == 4) {
          third_key_record = record;
        } else if (callback_count == 5) {
          fourth_key_record = record;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  keyboard_focus_fixture = nullptr;

  if (result != 0) {
    return 141;
  }
  if (callback_count != 5 || fixture.view.keyboard_key_count != 4 ||
      fixture.view.text_input_count != 1) {
    return 142;
  }
  if (first_key_record.route.target_element_id.has_value()) {
    return 143;
  }
  if (!second_key_record.route.target_element_id.has_value() ||
      *second_key_record.route.target_element_id != cgpui::ElementId{21}) {
    return 144;
  }
  if (!text_record.route.target_element_id.has_value() ||
      *text_record.route.target_element_id != cgpui::ElementId{21}) {
    return 145;
  }
  if (!third_key_record.route.target_element_id.has_value() ||
      *third_key_record.route.target_element_id != cgpui::ElementId{21}) {
    return 146;
  }
  if (fourth_key_record.route.target_element_id.has_value()) {
    return 147;
  }
  if (!fixture.view.second_key_saw_keyboard_focus ||
      !fixture.view.second_key_saw_keyboard_focus_element_owner ||
      !fixture.view.text_input_saw_keyboard_focus ||
      !fixture.view.text_input_saw_keyboard_focus_element_owner ||
      !fixture.view.third_key_saw_keyboard_focus ||
      !fixture.view.third_key_saw_keyboard_focus_element_owner) {
    return 148;
  }
  if (fixture.view.fourth_key_saw_keyboard_focus ||
      fixture.view.fourth_key_saw_keyboard_focus_element_owner ||
      fixture.view.last_keyboard_focus_element_owner_present) {
    return 149;
  }

  return 0;
}

int test_focus_handle_requests_releases_and_queries_focus() {
  RuntimeFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const cgpui::ElementId first_id{31};
  const cgpui::ElementId second_id{32};
  const cgpui::FocusHandle first = runtime.focus_handle(first_id);
  const cgpui::FocusHandle second = runtime.focus_handle(second_id);
  const cgpui::FocusHandle empty = runtime.focus_handle(cgpui::ElementId{});

  if (first.id() != first_id || first.empty() || !empty.empty()) {
    return 393;
  }
  if (first.contains(runtime) || first.focused(runtime) ||
      first.contains(runtime.input_state())) {
    return 394;
  }

  first.request(runtime);
  const cgpui::ViewInputState focused_input = runtime.input_state();
  if (!first.contains(focused_input) || !first.focused(focused_input) ||
      !first.contains(runtime) || !first.focused(runtime) ||
      second.contains(focused_input) || second.focused(focused_input)) {
    return 395;
  }

  second.release(runtime);
  if (!first.focused(runtime.input_state())) {
    return 396;
  }

  second.request(runtime);
  if (first.focused(runtime) || !second.focused(runtime)) {
    return 397;
  }

  first.release(runtime);
  if (!second.focused(runtime)) {
    return 398;
  }

  second.release(runtime);
  if (first.focused(runtime.input_state()) ||
      second.focused(runtime.input_state())) {
    return 399;
  }

  empty.request(runtime);
  if (empty.focused(runtime) || runtime.input_state().keyboard_focused) {
    return 400;
  }

  return 0;
}

int test_keyboard_focus_element_changes_request_style_invalidation() {
  RuntimeFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  runtime.request_keyboard_focus(cgpui::ElementId{51});
  cgpui::InvalidationState state = runtime.invalidation_state();
  if (!state.render || !state.layout || !state.paint) {
    return 407;
  }

  runtime.clear_invalidation();
  runtime.request_keyboard_focus(cgpui::ElementId{51});
  state = runtime.invalidation_state();
  if (state.render || state.layout || state.paint) {
    return 408;
  }

  runtime.request_keyboard_focus(cgpui::ElementId{52});
  state = runtime.invalidation_state();
  if (!state.render || !state.layout || !state.paint) {
    return 409;
  }

  runtime.clear_invalidation();
  runtime.release_keyboard_focus(cgpui::ElementId{51});
  state = runtime.invalidation_state();
  if (state.render || state.layout || state.paint) {
    return 410;
  }

  runtime.release_keyboard_focus(cgpui::ElementId{52});
  state = runtime.invalidation_state();
  if (!state.render || !state.layout || !state.paint) {
    return 411;
  }

  runtime.clear_invalidation();
  runtime.release_keyboard_focus(cgpui::ElementId{52});
  runtime.request_keyboard_focus(cgpui::ElementId{});
  state = runtime.invalidation_state();
  if (state.render || state.layout || state.paint) {
    return 412;
  }

  return 0;
}

int test_view_context_focus_handle_requests_releases_and_queries_focus() {
  RuntimeFixture fixture;
  keyboard_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_keyboard_focus_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{33};
  fixture.view.exercise_view_context_focus_handle = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  keyboard_focus_fixture = nullptr;

  if (result != 0) {
    return 401;
  }
  if (fixture.view.view_context_focus_handle_id != cgpui::ElementId{33} ||
      !fixture.view.view_context_focus_handle_empty) {
    return 402;
  }
  if (fixture.view.view_context_focus_handle_contains_before ||
      fixture.view.view_context_focus_handle_focused_before) {
    return 403;
  }
  if (!fixture.view.view_context_focus_handle_contains_after_request ||
      !fixture.view.view_context_focus_handle_focused_after_request ||
      fixture.view.view_context_focus_handle_other_contains) {
    return 404;
  }
  if (!fixture.view.view_context_focus_handle_still_focused_after_wrong_release ||
      !fixture.view.third_key_saw_keyboard_focus_element_owner) {
    return 405;
  }
  if (!fixture.view.view_context_focus_handle_released ||
      fixture.view.last_keyboard_focus_element_owner_present) {
    return 406;
  }

  return 0;
}

int test_view_context_focuses_and_blurs_element() {
  RuntimeFixture fixture;
  keyboard_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_keyboard_focus_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{22};
  fixture.view.exercise_view_context_focus_element_helpers = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  keyboard_focus_fixture = nullptr;

  if (result != 0) {
    return 250;
  }
  if (fixture.view.keyboard_key_count != 4 ||
      fixture.view.text_input_count != 1) {
    return 251;
  }
  if (!fixture.view.second_key_saw_keyboard_focus ||
      !fixture.view.second_key_saw_keyboard_focus_element_owner ||
      !fixture.view.text_input_saw_keyboard_focus_element_owner ||
      !fixture.view.third_key_saw_keyboard_focus_element_owner) {
    return 252;
  }
  if (fixture.view.fourth_key_saw_keyboard_focus ||
      fixture.view.fourth_key_saw_keyboard_focus_element_owner ||
      fixture.view.last_keyboard_focus_element_owner_present) {
    return 253;
  }

  return 0;
}


} // namespace

int main() {
  if (const int result = test_runtime_clicks_request_focus_for_focusable_elements(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_clicks_button_widget_focus_and_dispatch_action(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_tabs_focus_forward_over_enabled_focusable_elements(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_shift_tab_focuses_previous_enabled_focusable_element(); result != 0) {
    return result;
  }
  if (const int result =
          test_runtime_tabs_follow_explicit_tab_index_before_tree_order();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_tabs_skip_negative_tab_index_metadata();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_does_not_focus_disabled_focusable_elements(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_scrolls_bound_scroll_state_at_hit_element(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_scrolls_nearest_scroll_ancestor_for_child_hit(); result != 0) {
    return result;
  }
  if (const int result = test_view_can_request_and_release_keyboard_focus(); result != 0) {
    return result;
  }
  if (const int result = test_keyboard_focus_tracks_owner_view_id(); result != 0) {
    return result;
  }
  if (const int result = test_keyboard_focus_routes_to_owner_element(); result != 0) {
    return result;
  }
  if (const int result = test_focus_handle_requests_releases_and_queries_focus(); result != 0) {
    return result;
  }
  if (const int result = test_keyboard_focus_element_changes_request_style_invalidation(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_focus_handle_requests_releases_and_queries_focus(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_focuses_and_blurs_element(); result != 0) {
    return result;
  }
  return 0;
}
