#include "window_runtime_test_support.hpp"

namespace {
RuntimeFixture* key_binding_fixture = nullptr;

void dispatch_key_binding_sequence() {
  auto& callback = key_binding_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 79,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 83,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
}

void dispatch_view_context_key_binding_sequence() {
  auto& callback = key_binding_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 70,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 83,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
}

int test_runtime_dispatches_key_binding_actions() {
  RuntimeFixture fixture;
  key_binding_fixture = &fixture;
  fixture.app.on_run = &dispatch_key_binding_sequence;

  int action_count = 0;
  bool action_saw_keyboard_route = false;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.register_action(
      "app.save",
      [&](const cgpui::WindowRuntimeContext& context) {
        action_count += 1;
        action_saw_keyboard_route =
            context.event_route.has_value() &&
            context.event_route->event_kind == cgpui::EventKind::keyboard_key;
        return cgpui::EventResult::consumed_event();
      });
  runtime.bind_key(cgpui::KeyBinding{
      .key_code = 83,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true},
      .action_name = "app.save"});

  const int result = runtime.run(cgpui::WindowDescriptor{});
  key_binding_fixture = nullptr;

  if (result != 0) {
    return 156;
  }
  if (fixture.view.keyboard_key_count != 2 || action_count != 1) {
    return 157;
  }
  if (!action_saw_keyboard_route) {
    return 158;
  }
  const std::optional<cgpui::ActionDispatchResult> dispatch =
      runtime.last_action_dispatch();
  if (!dispatch.has_value() || dispatch->name != "app.save" ||
      !dispatch->handled || !dispatch->result.consumed ||
      dispatch->result.cancelled) {
    return 159;
  }

  return 0;
}

int test_view_context_binds_key_actions() {
  RuntimeFixture fixture;
  key_binding_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_context_key_binding_sequence;
  fixture.view.exercise_view_context_key_binding_helper = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  key_binding_fixture = nullptr;

  if (result != 0) {
    return 238;
  }
  if (fixture.view.keyboard_key_count != 2 ||
      fixture.view.view_context_key_binding_action_count != 1) {
    return 239;
  }
  if (!fixture.view.view_context_key_binding_action_saw_keyboard_route) {
    return 240;
  }
  const std::optional<cgpui::ActionDispatchResult> dispatch =
      runtime.last_action_dispatch();
  if (!dispatch.has_value() || dispatch->name != "view.save" ||
      !dispatch->handled || !dispatch->result.consumed ||
      dispatch->result.cancelled) {
    return 241;
  }

  return 0;
}

RuntimeFixture* text_input_routing_fixture = nullptr;

void dispatch_text_input_routing_sequence() {
  auto& callback = text_input_routing_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "hi"});
  callback(cgpui::KeyboardKey{
      .key_code = 85,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 86,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "!"});
}

int test_runtime_routes_text_input_to_focused_text_model() {
  RuntimeFixture fixture;
  text_input_routing_fixture = &fixture;
  fixture.app.on_run = &dispatch_text_input_routing_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;
  fixture.view.release_keyboard_focus_element_owner_on_third_key = true;

  cgpui::TextModel model;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);

  const int result = runtime.run(cgpui::WindowDescriptor{});
  text_input_routing_fixture = nullptr;

  if (result != 0) {
    return 160;
  }
  if (fixture.view.keyboard_key_count != 3 ||
      fixture.view.text_input_count != 2) {
    return 161;
  }
  if (model.text() != "hi") {
    return 162;
  }
  if (model.cursor() != 2) {
    return 164;
  }
  if (fixture.view.last_keyboard_focus_element_owner_present ||
      fixture.view.last_route_element_id.has_value()) {
    return 163;
  }

  return 0;
}

RuntimeFixture* text_input_widget_fixture = nullptr;

void dispatch_text_input_widget_sequence() {
  auto& callback = text_input_widget_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "!"});
  callback(cgpui::KeyboardKey{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.shift = true}});
}

int test_runtime_routes_text_input_widget_without_manual_binding() {
  RuntimeFixture fixture;
  text_input_widget_fixture = &fixture;
  fixture.app.on_run = &dispatch_text_input_widget_sequence;

  cgpui::TextModel model("abcd");
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId input_id =
      tree->set_root(cgpui::text_input(model)
                         .font(cgpui::FontDescriptor{.family = "Input"})
                         .font_size(20.0F)
                         .key("runtime-input")
                         .build());
  cgpui::TextInputElement* input =
      tree->find_as<cgpui::TextInputElement>(input_id);
  if (input == nullptr || !input->focusable() ||
      input->model() != &model) {
    return 310;
  }

  fixture.view.focused_keyboard_element_id = input_id;
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::MemoryClipboard clipboard;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_clipboard(&clipboard);
  runtime.set_element_tree(std::move(tree));
  runtime.bind_text_edit_action(cgpui::TextEditBinding{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed,
      .edit_action = cgpui::TextEditAction::move_previous});
  runtime.bind_text_edit_action(cgpui::TextEditBinding{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.shift = true},
      .edit_action = cgpui::TextEditAction::extend_previous});

  bool copied = false;
  bool cut = false;
  bool pasted = false;
  std::optional<cgpui::ImeCandidateRect> ime_rect;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.sequence == 4) {
          copied = context.runtime.copy_selection_to_clipboard();
          cut = context.runtime.cut_selection_to_clipboard();
          pasted = context.runtime.paste_clipboard_text();
          ime_rect = context.runtime.focused_text_ime_rect();
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  text_input_widget_fixture = nullptr;

  if (result != 0) {
    return 311;
  }
  if (runtime.focused_text_model() != &model) {
    return 312;
  }
  if (!copied || !cut || !pasted) {
    return 313;
  }
  const std::optional<std::string> clipboard_text = clipboard.read_text();
  if (!clipboard_text.has_value() || *clipboard_text != "d") {
    return 314;
  }
  if (model.text() != "abcd!" || model.cursor() != 4 ||
      !model.selection().collapsed) {
    return 315;
  }
  if (!ime_rect.has_value() || ime_rect->element_id != input_id ||
      ime_rect->byte_offset != 4 || ime_rect->rect.origin.x != 40.0F ||
      ime_rect->rect.size.height != 20.0F) {
    return 316;
  }

  return 0;
}

RuntimeFixture* text_pointer_selection_fixture = nullptr;

void dispatch_text_pointer_selection_sequence() {
  auto& callback = text_pointer_selection_fixture->window.callback;
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = true,
      .position = {26.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {6.0F, 5.0F}});
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = false,
      .position = {6.0F, 5.0F}});
}

int test_runtime_drags_text_input_pointer_selection() {
  RuntimeFixture fixture;
  text_pointer_selection_fixture = &fixture;
  fixture.app.on_run = &dispatch_text_pointer_selection_sequence;

  cgpui::TextModel model("abcd");
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId input_id =
      tree->set_root(cgpui::text_input(model).font_size(20.0F).build());

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  bool down_placed_cursor = false;
  bool drag_extended_selection = false;
  bool up_kept_selection = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::pointer_button &&
            record.sequence == 1) {
          down_placed_cursor =
              context.input.keyboard_focus_element_owner == input_id &&
              model.cursor() == 3 && model.selection().collapsed;
        } else if (record.event_kind == cgpui::EventKind::pointer_moved) {
          drag_extended_selection =
              model.selection_anchor() == 3 &&
              model.selection_head() == 1 &&
              model.selected_text() == std::string_view{"bc"};
        } else if (record.event_kind == cgpui::EventKind::pointer_button &&
                   record.sequence == 3) {
          up_kept_selection =
              model.selection_anchor() == 3 &&
              model.selection_head() == 1 &&
              model.selected_text() == std::string_view{"bc"};
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = true});
  text_pointer_selection_fixture = nullptr;

  if (result != 0) {
    return 328;
  }
  if (runtime.focused_text_model() != &model) {
    return 329;
  }
  if (!down_placed_cursor || !drag_extended_selection || !up_kept_selection) {
    return 330;
  }
  return 0;
}

void dispatch_text_pointer_word_selection_sequence() {
  auto& callback = text_pointer_selection_fixture->window.callback;
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = true,
      .click_count = 2,
      .position = {76.0F, 5.0F}});
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = false,
      .click_count = 2,
      .position = {76.0F, 5.0F}});
}

int test_runtime_double_click_selects_text_input_word() {
  RuntimeFixture fixture;
  text_pointer_selection_fixture = &fixture;
  fixture.app.on_run = &dispatch_text_pointer_word_selection_sequence;

  cgpui::TextModel model("alpha beta");
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId input_id =
      tree->set_root(cgpui::text_input(model).font_size(20.0F).build());

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  bool down_selected_word = false;
  bool up_kept_word_selection = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::pointer_button &&
            record.sequence == 1) {
          down_selected_word =
              context.input.keyboard_focus_element_owner == input_id &&
              model.selection_anchor() == 6 &&
              model.selection_head() == 10 &&
              model.selected_text() == std::string_view{"beta"};
        } else if (record.event_kind == cgpui::EventKind::pointer_button &&
                   record.sequence == 2) {
          up_kept_word_selection =
              model.selection_anchor() == 6 &&
              model.selection_head() == 10 &&
              model.selected_text() == std::string_view{"beta"};
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = true});
  text_pointer_selection_fixture = nullptr;

  if (result != 0) {
    return 338;
  }
  if (runtime.focused_text_model() != &model) {
    return 339;
  }
  if (!down_selected_word || !up_kept_word_selection) {
    return 340;
  }
  return 0;
}

int test_runtime_reports_focused_text_model() {
  RuntimeFixture fixture;

  cgpui::TextModel first("one");
  cgpui::TextModel second("two");
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  runtime.bind_text_model(cgpui::ElementId{21}, &first);
  runtime.bind_text_model(cgpui::ElementId{22}, &second);

  if (runtime.focused_text_model() != nullptr) {
    return 290;
  }

  runtime.request_keyboard_focus(cgpui::ElementId{21});
  if (runtime.focused_text_model() != &first) {
    return 291;
  }

  const cgpui::WindowRuntime& const_runtime = runtime;
  if (const_runtime.focused_text_model() != &first) {
    return 292;
  }

  runtime.request_keyboard_focus(cgpui::ElementId{22});
  cgpui::TextModel* focused = runtime.focused_text_model();
  if (focused != &second) {
    return 293;
  }
  focused->insert_text("!");
  if (second.text() != "two!") {
    return 294;
  }

  runtime.bind_text_model(cgpui::ElementId{22}, nullptr);
  if (runtime.focused_text_model() != nullptr) {
    return 295;
  }

  runtime.release_keyboard_focus(cgpui::ElementId{22});
  if (runtime.focused_text_model() != nullptr) {
    return 296;
  }

  return 0;
}

RuntimeFixture* ime_rect_fixture = nullptr;

void dispatch_focused_text_ime_rect_sequence() {
  auto& callback = ime_rect_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 85,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_reports_focused_text_ime_rect() {
  RuntimeFixture fixture;
  ime_rect_fixture = &fixture;
  fixture.app.on_run = &dispatch_focused_text_ime_rect_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{1};
  fixture.view.request_keyboard_focus_element_on_first_key = true;
  fixture.view.exercise_view_context_ime_rect = true;

  cgpui::TextModel model("abcd");
  model.set_selection(3, 3);
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId text_id =
      tree->set_root(std::make_unique<cgpui::TextElement>(
          &model,
          cgpui::Style{}.with_font_size(20.0F)));
  cgpui::TextElement* text_element = tree->find_as<cgpui::TextElement>(text_id);
  if (text_element == nullptr) {
    return 301;
  }
  text_element->set_layout_bounds(cgpui::Rect{
      .origin = {.x = 10.0F, .y = 12.0F},
      .size = {.width = 40.0F, .height = 20.0F}});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(text_id, &model);
  runtime.set_element_tree(std::move(tree));

  std::optional<cgpui::ImeCandidateRect> callback_rect;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.sequence == 2) {
          callback_rect = context.runtime.focused_text_ime_rect();
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  ime_rect_fixture = nullptr;

  if (result != 0) {
    return 302;
  }
  const std::optional<cgpui::ImeCandidateRect> runtime_rect =
      runtime.focused_text_ime_rect();
  if (!runtime_rect.has_value() || !callback_rect.has_value() ||
      !fixture.view.view_context_ime_rect_present) {
    return 303;
  }
  if (runtime_rect->element_id != text_id ||
      callback_rect->element_id != text_id ||
      fixture.view.view_context_ime_rect.element_id != text_id) {
    return 304;
  }
  if (runtime_rect->byte_offset != 3 || callback_rect->byte_offset != 3 ||
      fixture.view.view_context_ime_rect.byte_offset != 3) {
    return 305;
  }
  const cgpui::Rect rect = runtime_rect->rect;
  if (rect.origin.x != 30.0F || rect.origin.y != 0.0F ||
      rect.size.width != 1.0F || rect.size.height != 20.0F) {
    return 306;
  }
  if (callback_rect->rect.origin.x != rect.origin.x ||
      fixture.view.view_context_ime_rect.rect.origin.x != rect.origin.x) {
    return 307;
  }

  return 0;
}

int test_runtime_omits_focused_text_ime_rect_without_focus_or_layout() {
  RuntimeFixture fixture;
  cgpui::TextModel model("abc");
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId text_id =
      tree->set_root(std::make_unique<cgpui::TextElement>(
          &model,
          cgpui::Style{}.with_font_size(18.0F)));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(text_id, &model);
  runtime.set_element_tree(std::move(tree));

  if (runtime.focused_text_ime_rect().has_value()) {
    return 308;
  }
  runtime.request_keyboard_focus(text_id);
  if (runtime.focused_text_ime_rect().has_value()) {
    return 309;
  }
  return 0;
}

int test_runtime_accessibility_snapshot_uses_keyboard_focus_owner() {
  RuntimeFixture fixture;
  cgpui::TextModel model("query");
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(cgpui::into_element(cgpui::v_stack()));
  const cgpui::ElementId label_id =
      tree->append_child(root_id, cgpui::label("Search").build());
  const cgpui::ElementId input_id =
      tree->append_child(root_id, cgpui::text_input(model).build());
  (void)tree->layout_root(cgpui::LayoutInput{});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  cgpui::AccessibilityTreeSnapshot snapshot = runtime.accessibility_snapshot();
  const cgpui::AccessibilityNode* initial_input = snapshot.node(input_id);
  if (snapshot.root_element_id != root_id || snapshot.nodes.size() != 3 ||
      initial_input == nullptr || initial_input->focused) {
    return 322;
  }

  runtime.request_keyboard_focus(input_id);
  snapshot = runtime.accessibility_snapshot();
  const cgpui::AccessibilityNode* label = snapshot.node(label_id);
  const cgpui::AccessibilityNode* input = snapshot.node(input_id);
  if (label == nullptr || input == nullptr) {
    return 323;
  }
  if (label->role != cgpui::AccessibilityRole::label ||
      label->name != "Search" || label->focused) {
    return 324;
  }
  if (input->role != cgpui::AccessibilityRole::text_input ||
      input->name != "query" || input->text != "query" ||
      !input->focusable || !input->focused) {
    return 325;
  }

  return 0;
}

int test_runtime_sends_accessibility_snapshot_summary_to_platform_window() {
  RuntimeFixture fixture;
  cgpui::TextModel model("query");
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(cgpui::into_element(cgpui::v_stack()));
  const cgpui::ElementId label_id =
      tree->append_child(root_id, cgpui::label("Search").build());
  const cgpui::ElementId input_id =
      tree->append_child(root_id, cgpui::text_input(model).build());
  (void)label_id;
  (void)input_id;
  fixture.app.on_run = +[] {};

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  const int result = runtime.run(cgpui::WindowDescriptor{});
  if (result != 0) {
    return 326;
  }
  if (fixture.window.accessibility_update_count != 1 ||
      !fixture.window.last_accessibility_update.has_value()) {
    return 327;
  }

  const cgpui::PlatformAccessibilityTreeUpdate& update =
      *fixture.window.last_accessibility_update;
  if (update.root_element_id == 0 || update.node_count != 3 ||
      update.nodes.size() != 3 || update.focused_node_count != 0) {
    return 328;
  }

  const auto label = std::ranges::find_if(
      update.nodes,
      [](const cgpui::PlatformAccessibilityNodeUpdate& node) {
        return node.role == cgpui::PlatformAccessibilityRole::label &&
               node.name == "Search";
      });
  if (label == update.nodes.end() || label->element_id == 0 ||
      !label->parent_element_id.has_value() || label->child_count != 0 ||
      label->focused) {
    return 329;
  }

  const auto input = std::ranges::find_if(
      update.nodes,
      [](const cgpui::PlatformAccessibilityNodeUpdate& node) {
        return node.role == cgpui::PlatformAccessibilityRole::text_input &&
               node.name == "query" && node.text == "query";
      });
  if (input == update.nodes.end() || input->element_id == 0 ||
      !input->parent_element_id.has_value() || !input->focusable ||
      input->focused || input->child_count != 0 || input->value != "query") {
    return 330;
  }

  const cgpui::PlatformAccessibilityNodeUpdate& root = update.nodes.front();
  if (root.element_id != update.root_element_id ||
      root.role != cgpui::PlatformAccessibilityRole::generic ||
      root.parent_element_id.has_value() || root.child_count != 2) {
    return 331;
  }

  return 0;
}

RuntimeFixture* accessibility_live_update_fixture = nullptr;
cgpui::WindowRuntime* accessibility_live_update_runtime = nullptr;
cgpui::ElementId accessibility_live_update_input_id;
cgpui::TextModel* accessibility_live_update_model = nullptr;
bool accessibility_live_update_saw_focus_changed = false;

void dispatch_accessibility_live_update_sequence() {
  accessibility_live_update_runtime->request_keyboard_focus(
      accessibility_live_update_input_id);
  if (const auto& update =
          accessibility_live_update_fixture->window.last_accessibility_update;
      update.has_value()) {
    const auto focus_changed = std::ranges::find_if(
        update->live_updates,
        [](const cgpui::PlatformAccessibilityLiveUpdate& event) {
          return event.kind ==
                     cgpui::PlatformAccessibilityLiveUpdateKind::focus_changed &&
                 event.element_id == accessibility_live_update_input_id.value &&
                 event.focused;
        });
    accessibility_live_update_saw_focus_changed =
        focus_changed != update->live_updates.end();
  }
  accessibility_live_update_model->insert_text("!");
  accessibility_live_update_fixture->window.request_redraw();
}

int test_runtime_sends_accessibility_live_update_events_to_platform_window() {
  RuntimeFixture fixture;
  accessibility_live_update_fixture = &fixture;
  fixture.app.on_run = &dispatch_accessibility_live_update_sequence;
  accessibility_live_update_saw_focus_changed = false;

  cgpui::TextModel model("query");
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(cgpui::into_element(cgpui::v_stack()));
  const cgpui::ElementId input_id =
      tree->append_child(root_id, cgpui::text_input(model).build());
  (void)tree->layout_root(cgpui::LayoutInput{});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  accessibility_live_update_runtime = &runtime;
  accessibility_live_update_input_id = input_id;
  accessibility_live_update_model = &model;

  const int result = runtime.run(cgpui::WindowDescriptor{});
  accessibility_live_update_fixture = nullptr;
  accessibility_live_update_runtime = nullptr;
  accessibility_live_update_input_id = {};
  accessibility_live_update_model = nullptr;

  if (result != 0) {
    return 332;
  }
  if (fixture.window.accessibility_update_count != 3 ||
      !fixture.window.last_accessibility_update.has_value()) {
    return 333;
  }

  const cgpui::PlatformAccessibilityTreeUpdate& update =
      *fixture.window.last_accessibility_update;
  if (update.root_element_id != root_id.value || update.nodes.size() != 2 ||
      update.live_updates.size() != 2) {
    return 334;
  }

  const auto has_value_changed = std::ranges::find_if(
      update.live_updates,
      [input_id](const cgpui::PlatformAccessibilityLiveUpdate& event) {
        return event.kind ==
                   cgpui::PlatformAccessibilityLiveUpdateKind::value_changed &&
               event.element_id == input_id.value && event.value == "query!";
      });
  if (has_value_changed == update.live_updates.end()) {
    return 335;
  }

  const auto has_text_changed = std::ranges::find_if(
      update.live_updates,
      [input_id](const cgpui::PlatformAccessibilityLiveUpdate& event) {
        return event.kind ==
                   cgpui::PlatformAccessibilityLiveUpdateKind::text_changed &&
               event.element_id == input_id.value && event.text == "query!";
      });
  if (has_text_changed == update.live_updates.end()) {
    return 336;
  }

  if (!accessibility_live_update_saw_focus_changed) {
    return 337;
  }

  return 0;
}

int test_runtime_applies_focused_text_ime_rect_to_platform_window() {
  RuntimeFixture fixture;
  ime_rect_fixture = &fixture;
  fixture.app.on_run = &dispatch_focused_text_ime_rect_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{1};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("abcd");
  model.set_selection(3, 3);
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId text_id =
      tree->set_root(std::make_unique<cgpui::TextElement>(
          &model,
          cgpui::Style{}.with_font_size(20.0F)));
  cgpui::TextElement* text_element = tree->find_as<cgpui::TextElement>(text_id);
  if (text_element == nullptr) {
    return 317;
  }
  text_element->set_layout_bounds(cgpui::Rect{
      .origin = {.x = 10.0F, .y = 12.0F},
      .size = {.width = 40.0F, .height = 20.0F}});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(text_id, &model);
  runtime.set_element_tree(std::move(tree));
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.sequence == 2) {
          context.runtime.release_keyboard_focus(text_id);
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  ime_rect_fixture = nullptr;

  if (result != 0) {
    return 318;
  }
  std::optional<cgpui::ImeTextInputPlacement> applied_placement;
  for (const auto& placement : fixture.window.ime_placement_history) {
    if (placement.has_value()) {
      applied_placement = placement;
    }
  }
  if (fixture.window.ime_placement_count < 2 || !applied_placement.has_value()) {
    return 319;
  }
  const cgpui::ImeTextInputPlacement placement =
      *applied_placement;
  if (placement.byte_offset != 3 || placement.rect.origin.x != 30.0F ||
      placement.rect.origin.y != 0.0F || placement.rect.size.width != 1.0F ||
      placement.rect.size.height != 20.0F) {
    return 320;
  }

  if (fixture.window.last_ime_placement.has_value()) {
    return 321;
  }

  return 0;
}

int test_view_context_binds_text_model_to_element() {
  RuntimeFixture fixture;
  text_input_routing_fixture = &fixture;
  fixture.app.on_run = &dispatch_text_input_routing_sequence;
  fixture.view.exercise_view_context_text_model_binding = true;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};

  cgpui::TextModel model;
  fixture.view.view_context_bound_text_model = &model;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  text_input_routing_fixture = nullptr;

  if (result != 0) {
    return 224;
  }
  if (fixture.view.keyboard_key_count != 3 ||
      fixture.view.text_input_count != 2) {
    return 225;
  }
  if (model.text() != "hi!" || model.cursor() != 3) {
    return 226;
  }
  if (!fixture.view.text_input_saw_keyboard_focus_element_owner ||
      !fixture.view.last_route_element_id.has_value() ||
      *fixture.view.last_route_element_id != cgpui::ElementId{21}) {
    return 227;
  }

  return 0;
}

RuntimeFixture* focused_text_mutation_fixture = nullptr;

void dispatch_view_context_focused_text_mutation_sequence() {
  auto& callback = focused_text_mutation_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 85,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 86,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 87,
      .action = cgpui::KeyAction::pressed});
}

int test_view_context_mutates_focused_text_model() {
  RuntimeFixture fixture;
  focused_text_mutation_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_context_focused_text_mutation_sequence;
  fixture.view.exercise_view_context_focused_text_mutation = true;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};

  cgpui::TextModel model("hi");
  fixture.view.view_context_bound_text_model = &model;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  focused_text_mutation_fixture = nullptr;

  if (result != 0) {
    return 297;
  }
  if (!fixture.view.view_context_mutated_focused_text) {
    return 298;
  }
  if (!fixture.view.view_context_skipped_missing_focused_text) {
    return 299;
  }
  if (model.text() != "hi!" || model.cursor() != 3) {
    return 300;
  }

  return 0;
}

RuntimeFixture* view_context_element_tree_fixture = nullptr;

void dispatch_view_context_element_tree_sequence() {
  auto& callback = view_context_element_tree_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
}

int test_view_context_installs_element_tree() {
  RuntimeFixture fixture;
  view_context_element_tree_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_context_element_tree_sequence;
  fixture.view.exercise_view_context_element_tree_installation = true;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 700.0F, .height = 600.0F}));
  fixture.view.view_context_element_tree = std::move(tree);

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  std::optional<cgpui::ElementId> routed_element_id;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::pointer_moved) {
          routed_element_id = record.route.target_element_id;
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  view_context_element_tree_fixture = nullptr;

  if (result != 0) {
    return 228;
  }
  if (fixture.view.view_context_element_tree != nullptr ||
      runtime.element_tree() == nullptr || runtime.element_root() == nullptr) {
    return 229;
  }
  const std::optional<cgpui::Rect> bounds =
      runtime.element_root()->layout_bounds();
  if (!bounds.has_value() || bounds->size.width != 640.0F ||
      bounds->size.height != 480.0F) {
    return 230;
  }
  if (!routed_element_id.has_value() || *routed_element_id != root_id) {
    return 231;
  }

  return 0;
}

RuntimeFixture* text_edit_action_fixture = nullptr;

void dispatch_text_edit_action_sequence() {
  auto& callback = text_edit_action_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.shift = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 8,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 46,
      .action = cgpui::KeyAction::pressed});
}

void dispatch_view_context_text_edit_binding_sequence() {
  auto& callback = text_edit_action_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_routes_text_edit_actions_to_focused_text_model() {
  RuntimeFixture fixture;
  text_edit_action_fixture = &fixture;
  fixture.app.on_run = &dispatch_text_edit_action_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("abcd");
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.bind_text_edit_action(cgpui::TextEditBinding{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed,
      .edit_action = cgpui::TextEditAction::move_previous});
  runtime.bind_text_edit_action(cgpui::TextEditBinding{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.shift = true},
      .edit_action = cgpui::TextEditAction::extend_previous});
  runtime.bind_text_edit_action(cgpui::TextEditBinding{
      .key_code = 8,
      .action = cgpui::KeyAction::pressed,
      .edit_action = cgpui::TextEditAction::backspace});
  runtime.bind_text_edit_action(cgpui::TextEditBinding{
      .key_code = 46,
      .action = cgpui::KeyAction::pressed,
      .edit_action = cgpui::TextEditAction::delete_forward});

  const int result = runtime.run(cgpui::WindowDescriptor{});
  text_edit_action_fixture = nullptr;

  if (result != 0) {
    return 200;
  }
  if (fixture.view.keyboard_key_count != 5) {
    return 201;
  }
  if (model.text() != "ab") {
    return 202;
  }
  if (model.cursor() != 2 || !model.selection().collapsed) {
    return 203;
  }
  return 0;
}

int test_view_context_binds_text_edit_actions() {
  RuntimeFixture fixture;
  text_edit_action_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_context_text_edit_binding_sequence;
  fixture.view.exercise_view_context_text_edit_binding = true;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};

  cgpui::TextModel model("abcd");
  fixture.view.view_context_bound_text_model = &model;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  text_edit_action_fixture = nullptr;

  if (result != 0) {
    return 242;
  }
  if (fixture.view.keyboard_key_count != 2) {
    return 243;
  }
  if (model.text() != "abcd" || model.cursor() != 3 ||
      !model.selection().collapsed) {
    return 244;
  }
  if (!fixture.view.second_key_saw_keyboard_focus ||
      !fixture.view.second_key_saw_keyboard_focus_element_owner) {
    return 245;
  }

  return 0;
}

RuntimeFixture* clipboard_paste_fixture = nullptr;

void dispatch_clipboard_paste_sequence() {
  auto& callback = clipboard_paste_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_pastes_clipboard_text_into_focused_text_model() {
  RuntimeFixture fixture;
  clipboard_paste_fixture = &fixture;
  fixture.app.on_run = &dispatch_clipboard_paste_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("a");
  cgpui::MemoryClipboard clipboard;
  (void)clipboard.write_text("bc");

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.set_clipboard(&clipboard);

  bool pasted = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        pasted = context.runtime.paste_clipboard_text();
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  clipboard_paste_fixture = nullptr;

  if (result != 0) {
    return 204;
  }
  if (!pasted) {
    return 205;
  }
  if (model.text() != "abc" || model.cursor() != 3) {
    return 206;
  }
  return 0;
}

RuntimeFixture* clipboard_copy_fixture = nullptr;

void dispatch_clipboard_copy_sequence() {
  auto& callback = clipboard_copy_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_copies_focused_text_selection_to_clipboard() {
  RuntimeFixture fixture;
  clipboard_copy_fixture = &fixture;
  fixture.app.on_run = &dispatch_clipboard_copy_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("abcd");
  model.set_selection(1, 3);
  cgpui::MemoryClipboard clipboard;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.set_clipboard(&clipboard);

  bool copied = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        copied = context.runtime.copy_selection_to_clipboard();
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  clipboard_copy_fixture = nullptr;

  if (result != 0) {
    return 207;
  }
  if (!copied) {
    return 208;
  }
  const std::optional<std::string> copied_text = clipboard.read_text();
  if (!copied_text.has_value() || *copied_text != "bc") {
    return 209;
  }
  if (model.text() != "abcd" || model.selection().start != 1 ||
      model.selection().end != 3) {
    return 210;
  }
  return 0;
}

RuntimeFixture* clipboard_cut_fixture = nullptr;

void dispatch_clipboard_cut_sequence() {
  auto& callback = clipboard_cut_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_cuts_focused_text_selection_to_clipboard() {
  RuntimeFixture fixture;
  clipboard_cut_fixture = &fixture;
  fixture.app.on_run = &dispatch_clipboard_cut_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("abcd");
  model.set_selection(1, 3);
  cgpui::MemoryClipboard clipboard;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.set_clipboard(&clipboard);

  bool cut = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        cut = context.runtime.cut_selection_to_clipboard();
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  clipboard_cut_fixture = nullptr;

  if (result != 0) {
    return 211;
  }
  if (!cut) {
    return 212;
  }
  const std::optional<std::string> cut_text = clipboard.read_text();
  if (!cut_text.has_value() || *cut_text != "bc") {
    return 213;
  }
  if (model.text() != "ad" || model.cursor() != 1 ||
      !model.selection().collapsed) {
    return 214;
  }
  return 0;
}

RuntimeFixture* view_context_convenience_fixture = nullptr;

void dispatch_view_context_convenience_sequence() {
  auto& callback = view_context_convenience_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
}

int test_view_context_forwards_common_runtime_apis() {
  RuntimeFixture fixture;
  view_context_convenience_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_context_convenience_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;
  fixture.view.exercise_view_context_convenience = true;

  cgpui::TextModel model("abcd");
  model.set_selection(1, 3);
  cgpui::MemoryClipboard clipboard;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.set_clipboard(&clipboard);

  const int result = runtime.run(cgpui::WindowDescriptor{});
  view_context_convenience_fixture = nullptr;

  if (result != 0) {
    return 215;
  }
  if (!fixture.view.view_context_convenience_same_alias_type) {
    return 216;
  }
  if (!fixture.view.context_alias_same_view_context_type ||
      !fixture.view.context_alias_input_state_matches_snapshot ||
      !equal(fixture.view.context_alias_input_state.pointer_position,
             fixture.view.last_input_pointer_position)) {
    return 222;
  }
  if (!fixture.view.view_context_initial_invalidation.layout ||
      !fixture.view.view_context_initial_invalidation.paint ||
      !fixture.view.view_context_after_layout_request_invalidation.layout ||
      !fixture.view.view_context_after_layout_request_invalidation.paint ||
      fixture.view.view_context_after_clear_invalidation.layout ||
      fixture.view.view_context_after_clear_invalidation.paint ||
      !fixture.view.view_context_after_paint_request_invalidation.paint) {
    return 217;
  }
  if (!fixture.view.view_context_copied_selection ||
      !fixture.view.view_context_cut_selection ||
      !fixture.view.view_context_pasted_clipboard) {
    return 218;
  }
  const std::optional<std::string> clipboard_text = clipboard.read_text();
  if (!clipboard_text.has_value() || *clipboard_text != "bc") {
    return 219;
  }
  if (model.text() != "abcd" || model.cursor() != 3 ||
      !model.selection().collapsed) {
    return 220;
  }
  if (runtime.invalidation_state().layout || runtime.invalidation_state().paint) {
    return 221;
  }
  return 0;
}

RuntimeFixture* ime_composition_fixture = nullptr;

void dispatch_ime_composition_sequence() {
  auto& callback = ime_composition_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::ImeComposition{
      .phase = cgpui::ImeCompositionPhase::update,
      .text = "draft"});
  callback(cgpui::ImeComposition{
      .phase = cgpui::ImeCompositionPhase::commit,
      .text = "\xE4\xB8\xAD"});
}

int test_runtime_routes_ime_composition_to_focused_text_model() {
  RuntimeFixture fixture;
  ime_composition_fixture = &fixture;
  fixture.app.on_run = &dispatch_ime_composition_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model;
  std::optional<cgpui::EventDispatchRecord> update_record;
  std::optional<cgpui::EventDispatchRecord> commit_record;
  bool update_saw_composition = false;
  bool update_changed_text = false;
  bool commit_cleared_composition = false;
  bool commit_inserted_text = false;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind != cgpui::EventKind::ime_composition) {
          return;
        }
        if (!update_record.has_value()) {
          update_record = record;
          update_saw_composition =
              model.has_composition() &&
              model.composition_text() == std::string_view{"draft"};
          update_changed_text = !model.text().empty();
        } else {
          commit_record = record;
          commit_cleared_composition = !model.has_composition();
          commit_inserted_text =
              model.text() == std::string_view{"\xE4\xB8\xAD"};
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  ime_composition_fixture = nullptr;

  if (result != 0) {
    return 193;
  }
  if (fixture.view.keyboard_key_count != 1 ||
      fixture.view.ime_composition_count != 2) {
    return 194;
  }
  if (!update_record.has_value() || !commit_record.has_value()) {
    return 195;
  }
  if (update_record->route.event_kind != cgpui::EventKind::ime_composition ||
      commit_record->event_kind != cgpui::EventKind::ime_composition) {
    return 196;
  }
  if (!update_record->route.target_element_id.has_value() ||
      *update_record->route.target_element_id != cgpui::ElementId{21} ||
      !commit_record->route.target_element_id.has_value() ||
      *commit_record->route.target_element_id != cgpui::ElementId{21}) {
    return 197;
  }
  if (!fixture.view.ime_composition_saw_keyboard_focus ||
      !fixture.view.ime_composition_saw_keyboard_focus_element_owner) {
    return 198;
  }
  if (!update_saw_composition || update_changed_text ||
      !commit_cleared_composition || !commit_inserted_text) {
    return 199;
  }

  return 0;
}

RuntimeFixture* ime_delete_surrounding_fixture = nullptr;

void dispatch_ime_delete_surrounding_sequence() {
  auto& callback = ime_delete_surrounding_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::ImeDeleteSurroundingText{
      .before_length = 1,
      .after_length = 2});
}

int test_runtime_routes_ime_delete_surrounding_to_focused_text_model() {
  RuntimeFixture fixture;
  ime_delete_surrounding_fixture = &fixture;
  fixture.app.on_run = &dispatch_ime_delete_surrounding_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("abcdef");
  model.set_selection(3, 3);
  std::optional<cgpui::EventDispatchRecord> delete_record;
  bool delete_mutated_text = false;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind != cgpui::EventKind::ime_delete_surrounding_text) {
          return;
        }
        delete_record = record;
        delete_mutated_text =
            model.text() == std::string_view{"abf"} && model.cursor() == 2 &&
            model.selection().collapsed;
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  ime_delete_surrounding_fixture = nullptr;

  if (result != 0) {
    return 322;
  }
  if (fixture.view.keyboard_key_count != 1 ||
      fixture.view.ime_delete_surrounding_count != 1) {
    return 323;
  }
  if (!delete_record.has_value() ||
      delete_record->event_kind !=
          cgpui::EventKind::ime_delete_surrounding_text ||
      delete_record->route.event_kind !=
          cgpui::EventKind::ime_delete_surrounding_text) {
    return 324;
  }
  if (!delete_record->route.target_element_id.has_value() ||
      *delete_record->route.target_element_id != cgpui::ElementId{21}) {
    return 325;
  }
  if (!fixture.view.ime_delete_surrounding_saw_keyboard_focus ||
      !fixture.view.ime_delete_surrounding_saw_keyboard_focus_element_owner) {
    return 326;
  }
  if (!delete_mutated_text || !model.can_undo()) {
    return 327;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_runtime_dispatches_key_binding_actions(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_binds_key_actions(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_routes_text_input_to_focused_text_model(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_routes_text_input_widget_without_manual_binding(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_drags_text_input_pointer_selection(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_double_click_selects_text_input_word(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_reports_focused_text_model(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_reports_focused_text_ime_rect(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_omits_focused_text_ime_rect_without_focus_or_layout(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_accessibility_snapshot_uses_keyboard_focus_owner(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_sends_accessibility_snapshot_summary_to_platform_window(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_sends_accessibility_live_update_events_to_platform_window(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_applies_focused_text_ime_rect_to_platform_window(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_binds_text_model_to_element(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_mutates_focused_text_model(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_installs_element_tree(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_routes_text_edit_actions_to_focused_text_model(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_binds_text_edit_actions(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_pastes_clipboard_text_into_focused_text_model(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_copies_focused_text_selection_to_clipboard(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_cuts_focused_text_selection_to_clipboard(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_forwards_common_runtime_apis(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_routes_ime_composition_to_focused_text_model(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_routes_ime_delete_surrounding_to_focused_text_model(); result != 0) {
    return result;
  }
  return 0;
}
